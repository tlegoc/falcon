//
// Created by theo on 11/02/2025.
//

#include <protocol.h>

#include <chrono>
#include <utility>

#include <spdlog/spdlog.h>

void FalconClient::ConnectTo(const std::string &endpoint, uint16_t port) {
    mFalcon = Falcon::Connect(endpoint, port);
    if (!mFalcon)
    {
        spdlog::error("Failed to connect to server");
        if (mConnectionHandler) mConnectionHandler(false, {});
        return;
    }

    mFalcon->SetBlocking(false);
    mEndpoint = endpoint;
    mPort = port;

    ConnectHeader connectHeader{};
    connectHeader.version = PROTOCOL_VERSION;

    PacketBuilder builder(PacketType::CONNECT);
    builder.AddStruct(connectHeader);

    mFalcon->SendTo(mEndpoint, mPort, builder.GetData());

    std::array<char, 65535> buffer{};
    std::string from;
    bool timeout;
    PROTOCOL_TIMEOUT_HELPER(mFalcon, from, buffer, timeout);

    if (timeout)
    {
        spdlog::error("Connection timeout");

        mFalcon = nullptr;
        mEndpoint = "";
        mPort = 0;

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
        mEndpoint = "";
        mPort = 0;

        if (mConnectionHandler) mConnectionHandler(false, {});
        return;
    }

    ConnectAckHeader connectAckHeader{};
    reader.ReadHeader(connectAckHeader);

    mUuid = connectAckHeader.uuid;
    mReconnectToken = connectAckHeader.reconnectToken;

    if (mConnectionHandler) mConnectionHandler(true, mUuid);

    mLastSentPing = std::chrono::high_resolution_clock::now();
    mLastReceivedPing = std::chrono::high_resolution_clock::now();
}

void FalconClient::OnConnection(std::function<void(bool, uuid128_t)> handler) {
    mConnectionHandler = std::move(handler);
}

void FalconClient::OnDisconnect(std::function<void()> handler) {
    mDisconnectHandler = std::move(handler);
}

std::unique_ptr<Stream> FalconClient::CreateStream(bool reliable) {


    return nullptr;
}

void FalconClient::OnStreamCreated(std::function<void(bool)> handler) {
    mStreamCreatedHandler = std::move(handler);
}

void FalconClient::Tick() {
    if (!mFalcon) return;

    // Send ping to server
    if (std::chrono::high_resolution_clock::now() - mLastSentPing > mPingInterval) {
        PacketBuilder builder(PacketType::PING);

        PingHeader pingHeader{};
        pingHeader.uuid = mUuid;
        pingHeader.id = mLastPingId++;
        pingHeader.time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        builder.AddStruct(pingHeader);

        mFalcon->SendTo(mEndpoint, mPort, builder.GetData());

        mLastSentPing = std::chrono::high_resolution_clock::now();
    }

    // Listen to messages
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
        case PacketType::CONNECT:
        case PacketType::CONNECT_ACK:
            spdlog::info("Unsupported packet type: {}", packetHeader.type);
            break;
        case PacketType::PING: {
            HandlePingPacket(fromIp, fromPort, reader);
            break;
        }
    }

    if (mLastSentPing - mLastReceivedPing > std::chrono::seconds(PROTOCOL_DISCONNECT_TIMEOUT)) {
        mFalcon = nullptr;

        if (mDisconnectHandler) mDisconnectHandler();
    }
}

void FalconClient::HandlePingPacket(const std::string &ip, const uint16_t &port, PacketReader &reader) {
    PingHeader pingPacket{};
    reader.ReadHeader(pingPacket);

    mLastReceivedPing = std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::duration(pingPacket.time));
}

void FalconClient::Reconnect() {

}
