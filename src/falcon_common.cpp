#include "falcon.h"

int Falcon::SendTo(const std::string &to, uint16_t port, const std::span<const char> message)
{
    return SendToInternal(to, port, message);
}

int Falcon::ReceiveFrom(std::string& from, const std::span<char, 65535> message)
{
    return ReceiveFromInternal(from, message);
}

void Falcon::SplitIpString(const std::string &ip, std::string &outIp, uint16_t &outPort) {
    auto pos = ip.find_last_of(':');
    if (pos != std::string::npos) {
        outIp = ip.substr(0, pos);
        std::string portStr = ip.substr(++pos);
        outPort = static_cast<uint16_t>(std::stoi(portStr));
    }
}