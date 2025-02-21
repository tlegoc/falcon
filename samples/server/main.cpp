#include <iostream>

#include <falcon.h>
#include <protocol.h>

#include "spdlog/spdlog.h"

std::string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla quis vestibulum magna. In hac habitasse platea dictumst. Mauris vitae odio at libero tincidunt cursus. Mauris congue dolor in dapibus mattis. Proin egestas ultrices ultrices. Nullam sodales sapien nibh, eget malesuada erat iaculis et. Vestibulum id eros in risus condimentum dictum ac at elit. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec sollicitudin dictum ipsum ut iaculis. Nulla pulvinar, lectus cursus elementum malesuada, ligula augue consequat ante, et feugiat tellus nisi eu leo. Aenean in nisi ex. Nam rutrum, eros dapibus tristique lobortis, nisi lorem egestas lorem, nec mattis risus enim a ipsum. Vivamus eget sodales lorem. Pellentesque aliquam aliquet lacus, quis tristique quam varius nec. Vivamus scelerisque vestibulum lorem id vulputate. Donec vel nibh turpis. Cras luctus vestibulum libero, eu dictum purus eleifend et. Ut aliquet rutrum lorem vel auctor. Cras varius eget mauris at dictum. Mauris at faucibus nulla, eget faucibus arcu. Donec mi lectus, tempor quis egestas eget, gravida quis turpis. Pellentesque non tellus vestibulum, pulvinar libero sit amet, tincidunt felis. Maecenas eu scelerisque nisl. Aenean eleifend tortor eget aliquet molestie. Vivamus eleifend enim sit amet fermentum fermentum.Morbi tellus mauris, venenatis vel elit ac, scelerisque vulputate erat. Lorem ipsum dolor sit amet, consectetur adipiscing elit. In tempus, risus luctus massa nunc.";

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
                stream->SendData(text);
            }
        }
    }
}
