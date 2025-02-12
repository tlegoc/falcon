//
// Created by theo on 11/02/2025.
//

#pragma once

#include <memory>
#include <string>
#include <span>
#include <functional>
#include <falcon.h>
#include <array>

#include <uuid.h>

#define PROTOCOL_VERSION 1
#define PROTOCOL_TIMEOUT_SECONDS 5

enum PacketType {
    CONNECT = 0x00,
    CONNECT_ACK = 0x01,
    RECONNECT = 0x02,
    RECONNECT_ACK = CONNECT_ACK,
    DISCONNECT = 0x03,
    DATA = 0x04,
    DATA_ACK = 0x05,
    PING = 0x06
};

struct PacketHeader {
    uint8_t type;
    uint64_t size;
};

struct ConnectHeader {
    uint32_t version;
};

struct ConnectAckHeader {
    uuid128_t uuid;
    uuid128_t reconnectToken;
};

struct PingHeader {
    uuid128_t uuid;
    uint32_t id;
    uint64_t time;
};

struct streamid32_t {
    union {
        uint32_t id;
        struct {
            uint32_t flag: 1;
            uint32_t id: 3;
        } separatedId;
    };
};

struct DataHeader {
    uuid128_t uuid;
    streamid32_t streamId;
    uint32_t msgId;
    uint64_t size;
    uint8_t flags;
};

struct DataSplitHeader {
    uint32_t partId;
    uint32_t total;
};

struct DataAckHeader {
    uuid128_t uuid;
    uint32_t lastMsgId;
    std::array<uint8_t, 128> bitField; // should allow to validate 128 * 8 = 1024 messages
};

class PacketBuilder {
public:
    template<typename Header>
    void AddHeader(const Header &header) {
        AddData(std::span(reinterpret_cast<const char *>(&header), sizeof(Header)));
    }

    void AddData(std::span<const char> data) {
        mBuffer.insert(mBuffer.end(), data.begin(), data.end());
    }

    [[nodiscard]] std::span<const char> GetData() const {
        return mBuffer;
    }

    void Clear() {
        mBuffer.clear();
    }

private:
    std::vector<char> mBuffer;
};

class PacketReader {
public:
    PacketReader(std::span<const char> data) : mBuffer(data.begin(), data.end()) {}


    template<typename Header>
    bool ReadHeader(Header &header) {
        if (mBuffer.size() < sizeof(Header))
            return false;

        std::memcpy(&header, mBuffer.data(), sizeof(Header));
        mBuffer.erase(mBuffer.begin(), mBuffer.begin() + sizeof(Header));
        return true;
    }

    [[nodiscard]] std::span<const char> GetData() const {
        return mBuffer;
    }

    void Clear() {
        mBuffer.clear();
    }

private:
    std::vector<char> mBuffer;
};

class Stream {
public:
    void SendData(std::span<const char> Data);

    void OnDataReceived(std::span<const char> Data);
};

class FalconServer {
public:
    void Listen(uint16_t port);

    void OnClientConnected(std::function<void(uuid128_t)> handler);

    void OnClientDisconnected(std::function<void(uuid128_t)> handler);

    std::unique_ptr<Stream> CreateStream(uuid128_t client, bool reliable);

    void OnStreamCreated(std::function<void(uuid128_t, bool)> handler);

    void Tick();

private:
    std::unique_ptr<Falcon> mFalcon;

    std::function<void(uuid128_t)> mClientConnectedHandler;
    std::function<void(uuid128_t)> mClientDisconnectedHandler;
    std::function<void(uuid128_t, bool)> mStreamCreatedHandler;
};

class FalconClient : public Falcon {
public:
    void ConnectTo(const std::string &endpoint, uint16_t port);

    void OnConnection(std::function<void(bool, uuid128_t)> handler);

    void OnDisconnect(std::function<void()> handler);

    std::unique_ptr<Stream> CreateStream(bool reliable);

    void OnStreamCreated(std::function<void(bool)> handler);

    void Tick();

private:
    std::unique_ptr<Falcon> mFalcon;
    std::string mEndpoint;
    uint16_t mPort;
    uuid128_t mUuid;
    uuid128_t mReconnectToken;

    std::function<void(bool, uuid128_t)> mConnectionHandler;
    std::function<void()> mDisconnectHandler;
    std::function<void(bool)> mStreamCreatedHandler;
};