#include <iostream>

#include <falcon.h>
#include <protocol.h>

#include "spdlog/spdlog.h"

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");

    /*
    auto falcon = Falcon::Listen("127.0.0.1", 5555);
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    int recv_size = falcon->ReceiveFrom(from_ip, buffer);
    std::string ip = from_ip;
    uint16_t port = 0;
    auto pos = from_ip.find_last_of (':');
    if (pos != std::string::npos) {
        ip = from_ip.substr (0,pos);
        std::string port_str = from_ip.substr (++pos);
        port = atoi(port_str.c_str());
    }
    falcon->SendTo(ip, port, std::span {buffer.data(), static_cast<unsigned long>(recv_size)});
    return EXIT_SUCCESS;
     */

    FalconServer server;
    std::vector<std::shared_ptr<Stream>> streams;

    server.OnClientConnected([&](const uuid128_t &id) {
        spdlog::info("Client connected with id: {}", ToString(id));

        streams.push_back(server.CreateStream(id, false));
    });

    server.OnClientDisconnected([](const uuid128_t &id) {
        spdlog::info("Client disconnected with id: {}", ToString(id));
    });
    server.Listen(5555);

    auto lastDataSendTime = std::chrono::high_resolution_clock::now();
    while (true) {
        server.Tick();

        if (std::chrono::high_resolution_clock::now() - lastDataSendTime > std::chrono::seconds(1)) {
            lastDataSendTime = std::chrono::high_resolution_clock::now();

            spdlog::info("Sending data to {} streams", streams.size());

            for (auto &stream: streams) {
                stream->SendData("J'envoie un message au stream " + ToString(stream->GetStreamID()));
            }
        }
    }
}
