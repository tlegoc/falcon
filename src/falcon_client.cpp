//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

#include <chrono>
#include <utility>

#include <spdlog/spdlog.h>

void FalconClient::ConnectTo(const std::string &endpoint, uint16_t port) {
    mFalcon = Falcon::Connect(endpoint, port);
    if (!mFalcon) {
        spdlog::error("Failed to connect to server");
        if (mConnectionHandler) mConnectionHandler(false, {});
        return;
    }

    mFalcon->SetBlocking(false);
    mServerIp = endpoint;
    mServerPort = port;

    ConnectHeader connectHeader{};
    connectHeader.version = PROTOCOL_VERSION;

    PacketBuilder builder(PacketType::CONNECT);
    builder.AddStruct(connectHeader);

    mFalcon->SendTo(mServerIp, mServerPort, builder.GetData());

    std::array<char, 65535> buffer{};
    std::string from;
    bool timeout;
    PROTOCOL_TIMEOUT_HELPER(mFalcon, from, buffer, timeout);

    if (timeout) {
        spdlog::error("Connection timeout");

        mFalcon = nullptr;
        mServerIp = "";
        mServerPort = 0;

        if (mConnectionHandler) mConnectionHandler(false, {});
        return;
    }

    std::string fromIp;
    uint16_t fromPort;
    Falcon::SplitIpString(from, fromIp, fromPort);

    spdlog::info("Packet received from {}:{}", fromIp, fromPort);

    PacketReader reader(buffer);

    PacketHeader receivedHeader{};
    reader.ReadHeader(receivedHeader);

    if (receivedHeader.type != PacketType::CONNECT_ACK) {
        spdlog::error("Expected CONNECT_ACK, got {}", receivedHeader.type);

        mFalcon = nullptr;
        mServerIp = "";
        mServerPort = 0;

        if (mConnectionHandler) mConnectionHandler(false, {});
        return;
    }

    ConnectAckHeader connectAckHeader{};
    reader.ReadHeader(connectAckHeader);

    mUuid = connectAckHeader.uuid;
    mReconnectToken = connectAckHeader.reconnectToken;

    if (mConnectionHandler) mConnectionHandler(true, mUuid);

    mLastSentPing = Clock::now();
    mLastReceivedPing = Clock::now();
    mLastReceivedMessage = Clock::now();
}

void FalconClient::OnConnection(std::function<void(bool, uuid128_t)> handler) {
    mConnectionHandler = std::move(handler);
}

void FalconClient::OnDisconnect(std::function<void()> handler) {
    mDisconnectHandler = std::move(handler);
}

std::shared_ptr<Stream> FalconClient::CreateStream(bool reliable) {
    auto stream = std::make_shared<Stream>(static_cast<IStreamProvider *>(this), mUuid, reliable);

    mStreams[stream->GetStreamID()] = stream;

    spdlog::debug("Created stream with uuid: {}", ToString(stream->GetStreamID()));

    return stream;
}

void FalconClient::OnStreamCreated(std::function<void(std::shared_ptr<Stream>)> handler) {
    mStreamCreatedHandler = std::move(handler);
}

void FalconClient::SendStreamPacket(uuid128_t clientId, std::span<const char> data) {
    mFalcon->SendTo(mServerIp, mServerPort, data);
}

void FalconClient::Tick() {
    if (!mFalcon) return;

    // Listen to messages

    std::array<char, 65535> array{};
    std::span<const char> buffer;
    std::string from;
    int64_t size = mFalcon->ReceiveFrom(from, array);
    if (size > 0) {
        buffer = {array.data(), static_cast<size_t>(size)};

        std::string fromIp;
        uint16_t fromPort;
        Falcon::SplitIpString(from, fromIp, fromPort);

        if (!IsEndpointServer(fromIp, fromPort)) {
            spdlog::error("Received message from unknown source: {}:{}", fromIp, fromPort);
            return;
        }

        PacketReader reader(buffer);

        PacketHeader packetHeader{};
        reader.ReadHeader(packetHeader);

        switch (packetHeader.type) {
            default:
                spdlog::info("Unknown packet type: {}", packetHeader.type);
                break;
            case PacketType::PING: {
                HandlePingPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::DATA: {
                HandleDataPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::DATA_ACK: {
                HandleDataAckPacket(fromIp, fromPort, reader);
                break;
            }
            case PacketType::CONNECT:
            case PacketType::CONNECT_ACK:
            case PacketType::RECONNECT:
                spdlog::info("Unsupported packet type: {}", packetHeader.type);
                break;
        }

        mLastReceivedMessage = Clock::now();
    }

    // Send ping if necessary
    if (std::chrono::high_resolution_clock::now() - mLastSentPing > mPingInterval) {
        SendPingToServer();
    }

    // DEBUG Show rtt every second
// #ifdef _DEBUG
    static TimePoint lastPrintedRTT = Clock::now();
    if (Clock::now() - lastPrintedRTT > std::chrono::seconds(1)) {
        spdlog::debug("RTT: {}ms", std::chrono::duration_cast<std::chrono::milliseconds>(mRTT).count());
        lastPrintedRTT = Clock::now();
    }
// #endif

    // If the last received message was too long ago, then the server disconnected
    if (Clock::now() - mLastReceivedMessage > std::chrono::milliseconds(PROTOCOL_DISCONNECT_MILLISECONDS)) {
        spdlog::error("Connection timeout");
        if (mDisconnectHandler) mDisconnectHandler();
        mFalcon = nullptr;
    }
}

void FalconClient::HandlePingPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    PingHeader pingPacket{};
    reader.ReadHeader(pingPacket);

    if (pingPacket.uuid != mUuid) {
        spdlog::error("Received ping is not using our uuid");
        return;
    }

    if (pingPacket.id >= mCurrentPingId) {
        spdlog::error("Received ping with invalid id");
        return;
    }

    mLastReceivedPing = Clock::now();

    mRTT = mLastReceivedPing - TimePoint(Duration(pingPacket.time));
}

void FalconClient::HandleDataPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    DataHeader dataHeader{};
    reader.ReadHeader(dataHeader);

    if (dataHeader.uuid != mUuid) {
        spdlog::error("Received data ack is not using our uuid");
        return;
    }

    bool reliable = dataHeader.streamId[0] == 1; // UGLY but works and we dont have time

    // Retrieve or create stream if necessary
    std::shared_ptr<Stream> stream = nullptr;
    if (mStreams.contains(dataHeader.streamId)) {
        stream = mStreams[dataHeader.streamId];
    } else {
        stream = std::make_shared<Stream>(static_cast<IStreamProvider *>(this), mUuid, reliable);
        stream->SetStreamID(dataHeader.streamId);
        mStreams[dataHeader.streamId] = stream;

        if (mStreamCreatedHandler) mStreamCreatedHandler(stream);
    }

    auto fragmented = (dataHeader.flags & DataFlag::FRAGMENTED) == DataFlag::FRAGMENTED;
    if (fragmented) {
        DataSplitHeader splitHeader{};
        reader.ReadHeader(splitHeader);

        if (stream->HandleAck(dataHeader.msgId)) {
            stream->HandlePartialPacket(splitHeader, reader.GetRemainingData(), dataHeader.size);
        }
    } else {
        if (stream->HandleAck(dataHeader.msgId)) {
            stream->HandleDataReceived(reader.GetRemainingData());
        }
    }
}

void FalconClient::HandleDataAckPacket(const std::string &ip, uint16_t port, PacketReader &reader) {
    spdlog::debug("Received ack.");
    DataAckHeader dataAckHeader{};
    reader.ReadHeader(dataAckHeader);

    if (dataAckHeader.uuid != mUuid) {
        spdlog::error("Received data ack is not using our uuid");
        return;
    }

    if (mStreams.contains(dataAckHeader.streamId)) {
        auto stream = mStreams[dataAckHeader.streamId];

        auto receivedMessages = Stream::GetReceivedMessagesFromBitfield(dataAckHeader.lastMsgId,
                                                                        Stream::ConstCharToBitSet(
                                                                                dataAckHeader.bitField));

        stream->HandleMissingPackets(receivedMessages);
    }
}

bool FalconClient::IsEndpointServer(const std::string &ip, uint16_t port) {
    return ip == mServerIp && port == mServerPort;
}

void FalconClient::SendPingToServer() {
    mLastSentPing = Clock::now();

    PacketBuilder builder(PacketType::PING);

    PingHeader pingHeader{};
    pingHeader.uuid = mUuid;
    pingHeader.id = mCurrentPingId++;
    pingHeader.time = mLastSentPing.time_since_epoch().count();

    builder.AddStruct(pingHeader);

    mFalcon->SendTo(mServerIp, mServerPort, builder.GetData());
}
