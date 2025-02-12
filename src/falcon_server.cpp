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

//    spdlog::info("Packet received: {}, size {}", packetHeader.type, packetHeader.size);

    switch (packetHeader.type) {
        default:
            spdlog::info("Packet received: {}, size {}", packetHeader.type, packetHeader.size);
            break;
        case PacketType::CONNECT: {
            ConnectHeader connectPacket{};
            reader.ReadHeader(connectPacket);

            uuid128_t uuid = UuidGenerator::Generate();
            uuid128_t reconnectedToken = UuidGenerator::Generate();

            PacketHeader responseHeader{PacketType::CONNECT, sizeof(PacketHeader) + sizeof(ConnectHeader)};
            ConnectAckHeader responsePacket{uuid, reconnectedToken};
            PacketBuilder builder;
            builder.AddHeader(responseHeader);
            builder.AddHeader(responsePacket);

            mFalcon->SendTo(from, builder.GetData());
            break;
        }
            break;
    }
}
