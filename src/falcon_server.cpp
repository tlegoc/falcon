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

std::unique_ptr<Stream> FalconServer::CreateStream(uuid128_t client, bool reliable) {
    return nullptr;
}

void FalconServer::OnStreamCreated(std::function<void(uuid128_t, bool)> handler) {
    mStreamCreatedHandler = std::move(handler);
}

void FalconServer::Tick() {
    if (!mFalcon) return;

    std::array<char, 65535> buffer{};
    std::string from;
    if (mFalcon->ReceiveFrom(from, buffer) <= 0) return;

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
    }

    // Check if any client was not seen since a long time
}

void FalconServer::HandleConnectPacket(const std::string &ip, const uint16_t &port, PacketReader &reader) {
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

    if (mClientConnectedHandler) mClientConnectedHandler(uuid);
}

void FalconServer::HandlePingPacket(const std::string &ip, const uint16_t &port, PacketReader &reader) {
    PingHeader pingPacket{};
    reader.ReadHeader(pingPacket);

    // Update the player structure, etc
    // TODO

    // spdlog::info("Ping packet received (uuid: {})", ToString(pingPacket.uuid));

    // Send back a ping
    pingPacket.time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    PacketBuilder builder(PacketType::PING);
    builder.AddStruct(pingPacket);

    mFalcon->SendTo(ip, port, builder.GetData());
}
