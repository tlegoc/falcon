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

    PacketHeader packetHeader{};
    packetHeader.type = PacketType::CONNECT;
    packetHeader.size = sizeof(PacketHeader) + sizeof(ConnectHeader);
    ConnectHeader connectHeader{};
    connectHeader.version = PROTOCOL_VERSION;

    PacketBuilder builder;
    builder.AddHeader(packetHeader);
    builder.AddHeader(connectHeader);

    mFalcon->SendTo(mEndpoint, mPort, builder.GetData());

    std::array<char, 65535> buffer{};
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    std::string from;
    while (mFalcon->ReceiveFrom(from, buffer) <= 0) {
        if (std::chrono::high_resolution_clock::now() - now > std::chrono::seconds(PROTOCOL_TIMEOUT_SECONDS)) {
            spdlog::error("Connection timeout");

            mFalcon = nullptr;
            mEndpoint = "";
            mPort = 0;

            if (mConnectionHandler) mConnectionHandler(false, {});
            return;
        }
    }

    PacketReader reader(buffer);

    PacketHeader receivedHeader{};
    reader.ReadHeader(receivedHeader);
    ConnectAckHeader connectAckHeader{};
    reader.ReadHeader(connectAckHeader);

    mUuid = connectAckHeader.uuid;
    mReconnectToken = connectAckHeader.reconnectToken;

    if (mConnectionHandler) mConnectionHandler(true, mUuid);
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



//    std::array<char, 65535> buffer{};
//    std::string from;
//    mFalcon->ReceiveFrom(from, buffer);
}
