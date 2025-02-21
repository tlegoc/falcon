﻿//
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
#include <unordered_map>
#include <bitset>

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

enum DataFlag : uint8_t {
    NONE = 0,
    FRAGMENTED = 1 << 0
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
    uuid128_t streamId;
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
    uuid128_t streamId;
    uint32_t lastMsgId;
    std::bitset<1024> bitField; // should allow to validate 128 * 8 = 1024 messages
};

class PacketBuilder {
public:
    PacketBuilder(PacketType type) {
        mType = type;
        AddStruct(PacketHeader{});
    }

    template<typename Header>
    void AddStruct(const Header &header) {
        AddData(std::span(reinterpret_cast<const char *>(&header), sizeof(Header)));
    }

    void AddData(std::span<const char> data) {
        mBuffer.insert(mBuffer.end(), data.begin(), data.end());
    }

    // Will copy to buffer starting from data to specified end
    void AddData(const char *data, const uint64_t end) {
        size_t bufferSize = mBuffer.size();
        mBuffer.resize(bufferSize + end);
        std::memcpy(&mBuffer[bufferSize], data, end);
    }

    [[nodiscard]] std::span<const char> GetData() {
        PacketHeader header{mType, mBuffer.size()};

        // Updates the packet header contained at the beginning of mbuffer
        std::memcpy(mBuffer.data(), &header, sizeof(PacketHeader));

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

    [[nodiscard]] std::span<const char> GetRemainingData() const {
        return mBuffer;
    }

    void Clear() {
        mBuffer.clear();
    }

private:
    std::vector<char> mBuffer;
};

class IStreamProvider {
public:
    virtual ~IStreamProvider() = default;

    virtual void SendStreamPacket(uuid128_t clientId, std::span<const char> data) = 0;
};

#define MTU 1200 // Maximum Transmission Unit : 1200 octets

class Stream {
public:

    struct StreamPacket {
        uint32_t id;
        size_t size;
        std::span<const char> data;
    };

    struct FragmentedPacket
    {
        uint32_t total;
        std::vector<size_t> sizes;
        std::unordered_map<uint32_t, std::vector<char>> fragment;
    };

    explicit Stream(IStreamProvider *streamProvider, uuid128_t client, bool reliable);

    ~Stream() = default;

    Stream(const Stream &) = delete;

    Stream operator=(const Stream &) = delete;

    Stream(Stream &&) noexcept = delete;

    Stream operator=(Stream &&) noexcept = delete;

    void SendData(std::span<const char> data);

    void OnDataReceived(std::function<void(std::span<const char>)> function);

    void SendMissingPackets(const std::vector<uint32_t>& ackedList);

    void HandlePartialPacket(uint32_t packetId, DataSplitHeader header, std::span<const char> packetData, size_t size);

    void HandleDataReceived(std::span<const char> data, size_t size);

    static inline bool SequenceGreaterThan(uint16_t s1, uint16_t s2) {
        return ((s1 > s2) && (s1 - s2 <= 32768)) ||
               ((s1 < s2) && (s2 - s1 > 32768));
    }

    uint32_t GetLocalSequence() const { return mLocalSequence; }

    uint32_t GetRemoteSequence() const { return mRemoteSequence; }

    uuid128_t GetStreamID() const { return mStreamID; }

    void SetStreamID(uuid128_t id) { mStreamID = id;}

    static std::vector<uint32_t> GetReceivedMessagesFromBitfield(uint32_t lastMsg, std::bitset<1024> bitfield);

    static std::bitset<1024> GetBitFieldFromLastReceived(uint32_t lastMsg, std::vector<uint32_t> received);

private :
    IStreamProvider *mStreamProvider;
    uuid128_t mClientID;
    uuid128_t mStreamID;

    uint32_t mLocalSequence;
    uint32_t mRemoteSequence;

    std::array<uint8_t, 128> mAckHistory;
    std::unordered_map<uint32_t /*packet id*/, FragmentedPacket> mReceivedFragmentPacket;
    std::function<void(std::span<const char>)> mDataReceivedHandler;

    // Reliability part
    bool mReliability;
    std::vector<StreamPacket> mAckWaitList;
};

using Duration = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock, Duration>;

// Server socket
class FalconServer : public IStreamProvider {
public:
    void Listen(uint16_t port);

    void OnClientConnected(std::function<void(uuid128_t)> handler);

    void OnClientDisconnected(std::function<void(uuid128_t)> handler);

    std::shared_ptr<Stream> CreateStream(uuid128_t client, bool reliable);

    void OnStreamCreated(std::function<void(std::shared_ptr<Stream>)> handler);

    void SendStreamPacket(uuid128_t clientId, std::span<const char> data) override;

    void Tick();

    struct ClientEndpoint {
        std::string ip;
        uint16_t port;
    };

private:
    std::unique_ptr<Falcon> mFalcon;

    std::function<void(uuid128_t)> mClientConnectedHandler;
    std::function<void(uuid128_t)> mClientDisconnectedHandler;
    std::function<void(std::shared_ptr<Stream>)> mStreamCreatedHandler;

    std::unordered_map<uuid128_t, bool> mClients;
    std::unordered_map<uuid128_t, ClientEndpoint> mClientEndpoints;
    std::unordered_map<uuid128_t, uuid128_t> mReconnectTokens;
    std::unordered_map<uuid128_t, TimePoint> mLastReceivedPings;
    std::unordered_map<uuid128_t, std::unordered_map<uuid128_t, std::shared_ptr<Stream>>> mClientStreams;

    void HandleConnectPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    void HandlePingPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    void HandleDataPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    void HandleDataAckPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    bool IsEndpointValidForClient(const std::string &ip, uint16_t port, uuid128_t client);

    void CheckClientTimeout();
};

// Client socket
class FalconClient : public IStreamProvider {
public:
    void ConnectTo(const std::string &endpoint, uint16_t port);

    void OnConnection(std::function<void(bool, uuid128_t)> handler);

    void OnDisconnect(std::function<void()> handler);

    std::shared_ptr<Stream> CreateStream(bool reliable);

    void OnStreamCreated(std::function<void(std::shared_ptr<Stream>)> handler);

    void SendStreamPacket(uuid128_t clientId, std::span<const char> data) override;

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

    std::unordered_map<uuid128_t, std::shared_ptr<Stream>> mStreams;

    std::function<void(bool, uuid128_t)> mConnectionHandler;
    std::function<void()> mDisconnectHandler;
    std::function<void(std::shared_ptr<Stream>)> mStreamCreatedHandler;

    void HandlePingPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    void HandleDataPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    void HandleDataAckPacket(const std::string &ip, uint16_t port, PacketReader &reader);

    bool IsEndpointServer(const std::string &ip, uint16_t port);

    void SendPingToServer();
};