#include <iostream>

#include <falcon.h>
#include <protocol.h>

#include "spdlog/spdlog.h"

std::string text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam mauris risus, lacinia aliquam rutrum id, elementum molestie ipsum. Nulla congue finibus vestibulum. Donec aliquam sem ut tellus sodales, id auctor libero efficitur. Donec ut sollicitudin augue. Sed volutpat dictum velit facilisis dapibus. Curabitur non velit venenatis, dignissim arcu vitae, aliquam quam. Nullam in dignissim sapien, in auctor est. Nullam egestas ex eros, vel vestibulum dui vehicula eget.  Duis ut ipsum enim. Donec facilisis mauris quis pharetra gravida. Nulla eros mauris, tempor et dui nec, luctus hendrerit elit. Integer diam ipsum, congue ac congue non, porta in erat. Suspendisse potenti. Maecenas metus quam, volutpat eget semper aliquam, imperdiet vitae neque. Nulla id massa eu ex pulvinar efficitur eu ac lacus. Mauris id nibh magna. Fusce quis metus velit. Mauris turpis erat, bibendum non pellentesque a, mattis a elit.  Vivamus vehicula varius nibh vel euismod. Aliquam vestibulum ex sed lectus consectetur vehicula. Fusce fermentum finibus felis, vel suscipit justo placerat quis. Nunc venenatis ligula quis luctus condimentum. Cras mollis nisi ligula, non mollis justo laoreet sed. Donec congue mi leo, vel rutrum ex maximus vel. Sed sed velit eu ipsum auctor efficitur.  Praesent condimentum, lacus id tincidunt consectetur, dui sapien congue nisi, vel dictum nunc ipsum in urna. Nullam dapibus enim a mattis elementum. Suspendisse tellus magna, cursus at leo eu, ultrices facilisis lacus. Proin quis rhoncus augue, vel congue diam. Nam sed neque vel ex imperdiet tempus et non nibh. Cras tincidunt magna tellus, et pulvinar arcu lacinia in. Donec semper ante eget nulla blandit blandit. Nunc suscipit orci sit amet egestas mattis. Donec dolor nisi, placerat eu pretium ut, consectetur id augue. Pellentesque nisi diam, maximus ut arcu semper, volutpat ornare tellus. Nullam eu metus nec lectus malesuada lobortis id id nunc. Mauris sodales, augue eu molestie ultrices, urna quam aliquet metus, non euismod libero augue eu magna. Fusce in elit ut ante maximus fermentum at nec justo. Donec efficitur risus in mi egestas pellentesque. Vestibulum non cursus lacus.  Praesent molestie elementum ornare. Suspendisse nec enim tortor. In fermentum tellus in varius tincidunt. Suspendisse ullamcorper arcu risus, vel convallis ligula tempor ut. Nulla malesuada dapibus erat vel placerat. Morbi ut ligula viverra lorem egestas malesuada. Praesent dolor est, interdum at aliquam quis, venenatis eu dolor. Etiam justo tortor, hendrerit id justo eget, maximus euismod elit. Phasellus purus mi, semper nec orci in, dictum mollis purus. Vestibulum vitae nulla eget justo aliquet aliquet.  Donec ut quam eu augue fringilla facilisis. Vivamus risus magna, ultricies nec felis eget, bibendum sagittis felis. In iaculis porttitor congue. In at risus eu ex vulputate volutpat. Morbi eleifend velit ac tellus vehicula, quis ultricies metus accumsan. Vivamus sit amet tellus vitae ligula suscipit gravida. Sed tristique quam dignissim venenatis imperdiet. Ut vel quam libero. Curabitur malesuada massa at erat facilisis facilisis. Morbi euismod, velit et semper laoreet, urna nunc facilisis nibh, sed fermentum leo libero et elit. Nullam ultricies quis tellus a efficitur.  Fusce scelerisque efficitur nibh, vel luctus lorem porta ut. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vivamus consequat aliquam odio, id sodales elit dignissim a. Fusce quis sapien eu ipsum dignissim porta sed eget massa. Pellentesque vestibulum consequat metus, ut aliquet mi pretium id. Mauris ut ligula eu est tincidunt suscipit rutrum sed ex. Nulla nec lorem turpis. Etiam nec leo eros.  Suspendisse bibendum pulvinar quam, consectetur blandit neque vestibulum vel. Vivamus consequat mi quam, sed iaculis arcu mattis ac. Ut fringilla nec ligula nec placerat. Maecenas porta magna ut velit ullamcorper, et pharetra est posuere. Etiam aliquam magna sed ligula fermentum, in congue nibh hendrerit. Sed dictum nibh ac bibendum mollis. Aliquam risus nunc, faucibus sed tempor nec, placerat eget arcu. Nunc tempus mi eget nisl facilisis, eu auctor lacus efficitur. Nam tristique eget felis ac placerat. Suspendisse ac porta mauris, at bibendum diam. Nullam nisl ipsum, mollis et tincidunt non, sodales eu magna. Ut pharetra nec ipsum eget rhoncus. Aenean eget mollis velit, eu malesuada lorem.  Nunc rhoncus ultricies luctus. Vestibulum id molestie felis. Aliquam erat volutpat. Donec tristique lacinia felis quis mattis. Nulla facilisi. Quisque aliquet imperdiet tellus, ac pretium orci blandit a. Phasellus in sem vitae urna viverra pulvinar nec et arcu. Aenean id euismod sem, ac ullamcorper tortor. Pellentesque molestie velit nisl. Suspendisse sodales mi tellus, quis vestibulum augue tincidunt id. Vivamus aliquam libero non rutrum hendrerit. Etiam dolor mi, dictum ac pulvinar in, tincidunt et dui. Cras dignissim ligula sapien, et condimentum neque elementum in. In malesuada cursus leo, sit amet euismod quam facilisis egestas.  Aliquam et ipsum blandit, convallis turpis vel, condimentum arcu. Phasellus luctus lorem et odio iaculis molestie. Nam a neque magna. Pellentesque ipsum metus, mollis at leo eu, condimentum tristique lacus. Sed gravida interdum vulputate. Proin auctor, lorem sit amet laoreet laoreet, mauris ligula pellentesque tortor, vitae rutrum leo nisi sed ligula. In eros lectus, dignissim sed vestibulum nec, accumsan sed purus. Nunc tempus lacus sed libero pharetra varius. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut in elementum odio. Fusce fringilla congue purus, eu tempor lectus iaculis in. Quisque commodo libero et dui scelerisque, sed varius tortor tincidunt. Curabitur sagittis lectus eget libero pretium molestie. Duis libero ex, porta ut nibh at, consequat ullamcorper enim.  In hac habitasse platea dictumst. Donec efficitur bibendum diam, nec consectetur eros pretium ultricies. Praesent quis enim a elit congue aliquet. Quisque ultrices sit amet diam non cursus. Vestibulum placerat vulputate elementum. Quisque congue dolor id urna porttitor pulvinar. Morbi nisi leo, auctor tincidunt urna non, ultrices mattis tortor. Nulla nibh est, fermentum ac nibh in, porta tincidunt sem. Aenean dignissim finibus orci nec pretium. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Etiam sodales libero non sapien rutrum interdum eget id diam. Praesent venenatis auctor lacus, tristique venenatis nisl ornare ac. Quisque ut nulla sed ligula elementum egestas non sed metus. Proin auctor mi mi, eu facilisis urna luctus nec.  Etiam ac nulla eu magna mattis iaculis eget non neque. Nullam lobortis leo eu feugiat vulputate. Suspendisse lacinia tortor at egestas malesuada. Fusce at mauris mauris. Nam tempus leo pellentesque eros lobortis tincidunt. In leo ante, ultricies non pellentesque sit amet, pulvinar sit amet lorem. Donec tempus, mi non iaculis venenatis, nunc lacus finibus nulla, nec venenatis enim purus tempus lorem. Ut fermentum diam placerat porta sollicitudin. Maecenas et nisl sit amet lacus rutrum auctor. Etiam sollicitudin sed diam sed rutrum. Integer iaculis mi vel semper scelerisque.  Etiam pulvinar a ante nec varius. In scelerisque, ipsum in consectetur efficitur, ipsum mi tincidunt nibh, et lobortis erat ipsum sed lacus. Fusce laoreet eget nisl eu maximus. Integer sodales malesuada ornare. Pellentesque ipsum ipsum, sollicitudin ut sodales placerat, cursus ac ligula. Mauris pellentesque lorem vel dolor dapibus mollis. Aenean pharetra vehicula sem vel viverra. In luctus mi ac condimentum convallis. Praesent efficitur tempor ex, eget eleifend risus dapibus vitae. Nulla in libero eu leo maximus maximus. Suspendisse eget pellentesque nunc.  Quisque porttitor ipsum eget velit imperdiet hendrerit. Donec sagittis lorem vel ligula aliquet, id laoreet enim porttitor. Mauris mattis vehicula leo, sit amet accumsan elit gravida at. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Morbi fringilla, sem sed dapibus faucibus, dolor diam bibendum dolor, quis dapibus diam diam ut arcu. Quisque nibh metus, tempor eget cursus eu, ultricies a lacus. Nulla tristique imperdiet risus, ut finibus dolor porta quis. Mauris eget auctor nibh. Suspendisse ac neque tincidunt, mollis ipsum at, gravida lacus.  Etiam lobortis sit amet risus nec mattis. Nam cursus mauris enim, id faucibus nunc imperdiet quis. Quisque sollicitudin elit in fermentum maximus. Maecenas dignissim, tellus at dapibus mattis, mauris ipsum varius orci, a blandit augue massa id erat. Ut lacus metus, tincidunt non mollis ac, vestibulum id lectus. Nulla pretium dolor eget dui sollicitudin faucibus. Nulla facilisi. Proin rhoncus sem auctor, pellentesque lorem non, porta lorem. Morbi faucibus, tellus eget finibus rhoncus, nisi dolor dignissim odio, id egestas ligula diam ac libero.  Pellentesque id metus justo. Nulla turpis ante, ornare iaculis sem sit amet, cursus blandit dolor. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nam luctus aliquam dictum. Suspendisse eu tempor dui. Etiam nec commodo diam. Curabitur vel libero eget mi gravida hendrerit. Morbi in scelerisque odio, vel finibus ipsum.  Sed mollis fermentum quam, et consectetur justo facilisis id. Vivamus lacinia mauris a sapien egestas maximus. Suspendisse vel orci et leo rhoncus efficitur. Quisque at congue nisi. Morbi nisl nibh, faucibus id ultrices quis, tristique eget nibh. Phasellus vel ex arcu. Etiam tincidunt nisl vel suscipit sollicitudin. Pellentesque tincidunt lorem in felis lacinia, congue pellentesque nulla hendrerit. Pellentesque in tristique dui. Integer ut eros non eros interdum porttitor non nec neque. In elementum metus sodales finibus ornare. Nullam cursus vestibulum sapien.  Pellentesque mollis ante ut convallis sagittis. Nullam sit amet ante nec erat congue ullamcorper id id lectus. Vestibulum ante ipsum primis in faucibus orci luctus et vel.";

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
