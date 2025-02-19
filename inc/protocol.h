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
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#include <uuid.h>

#define PROTOCOL_VERSION 1
#define PROTOCOL_TIMEOUT_MILLISECONDS 5000
#define PROTOCOL_DISCONNECT_MILLISECONDS 1000

#define PROTOCOL_TIMEOUT_HELPER(socket, from, buffer, timeout) do { \
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(); \
    timeout = false; \
    while (socket->ReceiveFrom(from, buffer) <= 0) { \
    if (std::chrono::high_resolution_clock::now() - now > std::chrono::milliseconds(PROTOCOL_TIMEOUT_MILLISECONDS)) { \
         timeout = true;                                            \
         break;                                                                \
    } \
    } } while(0)

enum PacketType : uint8_t {
    CONNECT = 0x00,
    CONNECT_ACK = 0x01,
    RECONNECT = 0x02,
    RECONNECT_ACK = CONNECT_ACK,
    DATA = 0x03,
    DATA_ACK = 0x04,
    PING = 0x05
};

enum DataFlag {
    FRAGMENTED = 0x00
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

struct ReconnectHeader {
    uuid128_t uuid;
    uuid128_t reconnectToken;
};

struct PingHeader {
    uuid128_t uuid;
    uint32_t id;
    uint64_t time;
};

using streamid32_t = uint32_t;

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
    // streamid32_t streamId;
    uint32_t lastMsgId;
    std::array<uint8_t, 128> bitField; // should allow to validate 128 * 8 = 1024 messages
};

class PacketBuilder {
public:
    PacketBuilder(PacketType type) {
        mType = type;
    }

    template<typename Header>
    void AddStruct(const Header &header) {
        AddData(std::span(reinterpret_cast<const char *>(&header), sizeof(Header)));
    }

    void AddData(std::span<const char> data) {
        mBuffer.insert(mBuffer.end(), data.begin(), data.end());
    }

    [[nodiscard]] std::span<const char> GetData() {
        PacketHeader header{mType, mBuffer.size() + sizeof(PacketHeader)};
        auto data = std::span(reinterpret_cast<const char *>(&header), sizeof(PacketHeader));
        mBuffer.insert(mBuffer.begin(), data.begin(), data.end());
        return mBuffer;
    }

    void Clear() {
        mBuffer.clear();
    }

private:
    std::vector<char> mBuffer;
    uint8_t mType;
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

class IStreamProvider
{
    virtual ~IStreamProvider() = 0;

    void SendStreamPacket(streamid32_t id, std::span<const char> data)
    {

    }

    virtual void HandleData(std::span<const char> data) = 0;

    virtual void HandleDataAck(std::span<const char> data) = 0;
};

#define MTU 1200 // Maximum Transmission Unit : 1200 octets

class Stream {
public:

    struct StreamPacket {
        uint16_t id;
        std::span<const char> data;
    };

    explicit Stream(std::shared_ptr<Falcon> sock, uuid128_t client, bool reliable);

    ~Stream() = default;

    Stream(const Stream &) = delete;
    Stream operator=(const Stream &) = delete;

    Stream(Stream &&) noexcept = delete;
    Stream operator=(Stream &&) noexcept = delete;

    static streamid32_t streamCounter;

    void SendData(std::span<const char> data);

    void OnDataReceived(std::span<const char> data);

    inline bool SequenceGreaterThan(uint16_t s1, uint16_t s2) {
        return ((s1 > s2) && (s1 - s2 <= 32768)) ||
               ((s1 < s2) && (s2 - s1 > 32768));
    }

    uint16_t GetLocalSequence() const { return mLocalSequence; };

    int GetRemoteSequence() const { return mRemoteSequence; };

    streamid32_t GetStreamID() const { return mStreamID; };

private :
    std::shared_ptr<Falcon> mSocket;
    uuid128_t clientID;
    streamid32_t mStreamID;

    uint16_t mLocalSequence;
    uint16_t mRemoteSequence;

    std::array<uint8_t, 128> mAckHistory;
    std::vector<std::span<const char>> mReceivedFragmentPacket;

    // Reliability part
    bool mReliability;
    std::vector<StreamPacket> mAckWaitList;
};

using Duration = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock, Duration>;

// Server socket
class FalconServer {
public:
    void Listen(uint16_t port);

    void OnClientConnected(std::function<void(uuid128_t)> handler);

    void OnClientDisconnected(std::function<void(uuid128_t)> handler);

    std::unique_ptr<Stream> CreateStream(uuid128_t client, bool reliable);

    void OnStreamCreated(std::function<void(uuid128_t, bool)> handler);

    void Tick();

    struct ClientEndpoint
    {
        std::string ip;
        uint16_t port;
    };

private:
    std::unique_ptr<Falcon> mFalcon;

    std::function<void(uuid128_t)> mClientConnectedHandler;
    std::function<void(uuid128_t)> mClientDisconnectedHandler;
    std::function<void(uuid128_t, bool)> mStreamCreatedHandler;

    std::unordered_map<uuid128_t, bool> mClients;
    std::unordered_map<uuid128_t, ClientEndpoint> mClientEndpoints;
    std::unordered_map<uuid128_t , uuid128_t> mReconnectTokens;
    std::unordered_map<uuid128_t , TimePoint> mLastReceivedPings;

    void HandleConnectPacket(const std::string&ip, uint16_t port, PacketReader &reader);
    void HandlePingPacket(const std::string&ip, uint16_t port, PacketReader &reader);

    bool IsEndpointValidForClient(const std::string& ip, uint16_t port, uuid128_t client);

    void CheckClientTimeout();
};

// Client socket
class FalconClient {
public:
    void ConnectTo(const std::string &endpoint, uint16_t port);

    void Reconnect();

    void OnConnection(std::function<void(bool, uuid128_t)> handler);

    void OnDisconnect(std::function<void()> handler);

    std::unique_ptr<Stream> CreateStream(bool reliable);

    void OnStreamCreated(std::function<void(bool)> handler);

    void Tick();

private:
    std::unique_ptr<Falcon> mFalcon;
    std::string mServerIp;
    uint16_t mServerPort;
    uuid128_t mUuid;
    uuid128_t mReconnectToken;
    TimePoint mLastReceivedMessage;

    uint32_t mCurrentPingId;
    Duration mPingInterval = Duration(50);
    TimePoint mLastReceivedPing;
    TimePoint mLastSentPing;
    Duration mRTT;

    std::function<void(bool, uuid128_t)> mConnectionHandler;
    std::function<void()> mDisconnectHandler;
    std::function<void(bool)> mStreamCreatedHandler;

    void HandlePingPacket(const std::string& ip, uint16_t port, PacketReader &reader);

    bool IsEndpointServer(const std::string& ip, uint16_t port);

    void SendPingToServer();
};