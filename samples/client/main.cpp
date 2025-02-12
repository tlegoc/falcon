#include <iostream>

#include <falcon.h>

#include <spdlog/spdlog.h>

#include <protocol.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");

    /*
    auto falcon = Falcon::Connect("127.0.0.1", 5556);
    std::string message = "Hello World!";
    std::span data(message.data(), message.size());
    falcon->SendTo("127.0.0.1", 5555, data);

    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    falcon->ReceiveFrom(from_ip, buffer);
    return EXIT_SUCCESS;
     */

    FalconClient client;
    client.OnConnection([](bool success, uuid128_t id) {
        if (success) {
            spdlog::info("Connected to server with id: {}", ToString(id));
        } else {
            spdlog::error("Failed to connect to server");
        }
    });
    client.ConnectTo("127.0.0.1", 5555);
}
