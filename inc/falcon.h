#pragma once

#include <memory>
#include <string>
#include <span>
#include <functional>

#ifdef WIN32
using SocketType = unsigned int;
#else
using SocketType = int;
#endif

class Falcon {
public:
    static std::unique_ptr<Falcon> Listen(const std::string &endpoint, uint16_t port);

    static std::unique_ptr<Falcon> Connect(const std::string &serverIp, uint16_t port);

    Falcon();

    ~Falcon();

    Falcon(const Falcon &) = delete;

    Falcon &operator=(const Falcon &) = delete;

    Falcon(Falcon &&) = default;

    Falcon &operator=(Falcon &&) = default;

    int SendTo(const std::string &to, uint16_t port, std::span<const char> message);

    int ReceiveFrom(std::string &from, std::span<char, 65535> message);

    int SetBlocking(bool shouldBlock);

private:
    int SendToInternal(const std::string &to, uint16_t port, std::span<const char> message);

    int ReceiveFromInternal(std::string &from, std::span<char, 65535> message);

    SocketType m_socket;
};