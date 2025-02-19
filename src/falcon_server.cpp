//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

#include <utility>

#include <spdlog/spdlog.h>

void FalconServer::Listen(uint16_t port) {
    mFalcon = Falcon::Listen("127.0.0.1", port);
    if (!mFalcon) {
        spdlog::error("Failed to listen on port {}", port);
        return;
    }
    mFalcon->SetBlocking(false);
}

void FalconServer::OnClientConnected(std::function<void(uuid128_t)> handler) {
    mClientConnectedHandler = std::move(handler);
}

void FalconServer::OnClientDisconnected(std::function<void(uuid128_t)> handler) {
    mClientDisconnectedHandler = std::move(handler);
}

std::shared_ptr<Stream> FalconServer::CreateStream(uuid128_t client, bool reliable) {
    auto stream = std::make_shared<Stream>(static_cast<IStreamProvider *>(this), client, reliable);

    mClientStreams[client][stream->GetStreamID()] = stream;

    spdlog::debug("Created stream with uuid: {} for client {}", ToString(stream->GetStreamID()), ToString(client));

    return stream;
}

void FalconServer::OnStreamCreated(std::function<void(std::shared_ptr<Stream>)> handler) {
    mStreamCreatedHandler = std::move(handler);
}

void FalconServer::Tick() {
    if (!mFalcon) return;

    std::array<char, 65535> buffer{};
    std::string from;
    if (mFalcon->ReceiveFrom(from, buffer) > 0) {
        PacketReader reader(buffer);

        PacketHeader packetHeader{};
        reader.ReadHeader(packetHeader);

        std::string fromIp;
        uint16_t fromPort;
        Falcon::SplitIpString(from, fromIp, fromPort);

        // spdlog::info("Packet received from {}:{}: {}, size {}", fromIp, fromPort, packetHeader.type, packetHeader.size);

        switch (packetHeader.type) {
            default:
                spdlog::info("Unknown packet type: {}", packetHeader.type);
                break;
            case PacketType::CONNECT: {
                HandleConnectPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::PING: {
                HandlePingPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::DATA: {
                HandleDataPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::DATA_ACK:
                break;
            case PacketType::RECONNECT:
            case PacketType::CONNECT_ACK:
                spdlog::info("Unsupported packet type: {}", packetHeader.type);
                break;
        }
    }

    // Check if any client was not seen since a long time
    CheckClientTimeout();
}

void FalconServer::HandleConnectPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    ConnectHeader connectPacket{};
    reader.ReadHeader(connectPacket);

    if (connectPacket.version != PROTOCOL_VERSION) {
        spdlog::error("Client joined with invalid protocol version: {}", connectPacket.version);
        return;
    }

    uuid128_t uuid = UuidGenerator::Generate();
    uuid128_t reconnectedToken = UuidGenerator::Generate();

    ConnectAckHeader responsePacket{uuid, reconnectedToken};
    PacketBuilder builder(PacketType::CONNECT_ACK);
    builder.AddStruct(responsePacket);

    mFalcon->SendTo(ip, port, builder.GetData());

    mClients[uuid] = true; // status (connected if true)
    mClientEndpoints[uuid] = {ip, port}; // Will change on reconnect
    mReconnectTokens[reconnectedToken] = uuid;
    mLastReceivedPings[uuid] = Clock::now();

    if (mClientConnectedHandler) mClientConnectedHandler(uuid);
}

void FalconServer::HandlePingPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    PingHeader pingPacket{};
    reader.ReadHeader(pingPacket);

    if (!IsEndpointValidForClient(ip, port, pingPacket.uuid)) {
        spdlog::error("Received ping packet from invalid endpoint");
        return;
    }

    mLastReceivedPings[pingPacket.uuid] = Clock::now();

    PacketBuilder builder(PacketType::PING);
    builder.AddStruct(pingPacket);

    mFalcon->SendTo(ip, port, builder.GetData());
}

void FalconServer::CheckClientTimeout() {
    for (const auto &[uuid, connected]: mClients) {
        if (!connected) continue;

        auto lastPing = mLastReceivedPings[uuid];
        if (Clock::now() - lastPing > std::chrono::milliseconds(PROTOCOL_DISCONNECT_MILLISECONDS)) {
            if (mClientDisconnectedHandler) mClientDisconnectedHandler(uuid);
            mClients[uuid] = false;
        }
    }
}

bool FalconServer::IsEndpointValidForClient(const std::string &ip, uint16_t port, uuid128_t client) {
    auto endpoint = mClientEndpoints[client];

    return endpoint.ip == ip && endpoint.port == port;
}

void FalconServer::SendStreamPacket(uuid128_t clientId, std::span<const char> data) {
    if (!mClients[clientId])
    {
        spdlog::warn("Sending data to a disconnected client {}!!", ToString(clientId));
        return;
    }

    auto endpoint = mClientEndpoints[clientId];

    mFalcon->SendTo(endpoint.ip, endpoint.port, data);
}

void FalconServer::HandleDataPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    DataHeader dataHeader{};
    reader.ReadHeader(dataHeader);

    if (!IsEndpointValidForClient(ip, port, dataHeader.uuid)) {
        spdlog::error("Received data packet from invalid endpoint");
        return;
    }

    bool reliable = dataHeader.streamId[0] == 1;

    std::shared_ptr<Stream> stream = nullptr;
    if (mClientStreams[dataHeader.uuid].contains(dataHeader.streamId)) {
        stream = mClientStreams[dataHeader.uuid][dataHeader.streamId];
    } else {
        stream = CreateStream(dataHeader.uuid, reliable);
        if (mStreamCreatedHandler) mStreamCreatedHandler(stream);
    }

    auto fragmented = (dataHeader.flags & DataFlag::FRAGMENTED) == DataFlag::FRAGMENTED;
    if (fragmented) {
        DataSplitHeader splitHeader{};
        reader.ReadHeader(splitHeader);

        stream->HandlePartialPacket(dataHeader.msgId, splitHeader, reader.GetRemainingData());
    } else {
        stream->HandleDataReceived(reader.GetRemainingData());
    }
}

