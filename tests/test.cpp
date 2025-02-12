#include <string>
#include <array>
#include <span>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "falcon.h"
#include "uuid.h"

TEST_CASE("Can Listen", "[falcon]") {
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    REQUIRE(receiver != nullptr);
}

TEST_CASE("Can Connect", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    REQUIRE(sender != nullptr);
}

TEST_CASE("Can Send To", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    std::string message = "Hello World!";
    std::span data(message.data(), message.size());
    int bytes_sent = sender->SendTo("127.0.0.1", 5555, data);
    REQUIRE(bytes_sent == message.size());
}

TEST_CASE("Can Receive From", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    std::string message = "Hello World!";
    std::span data(message.data(), message.size());
    int bytes_sent = sender->SendTo("127.0.0.1", 5555, data);
    REQUIRE(bytes_sent == message.size());
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    int byte_received = receiver->ReceiveFrom(from_ip, buffer);

    REQUIRE(byte_received == message.size());
    REQUIRE(std::equal(buffer.begin(),
                       buffer.begin() + byte_received,
                       message.begin(),
                       message.end()));
    REQUIRE(from_ip == "127.0.0.1:5556");
}

TEST_CASE("Uuid equal operator", "[uuid]")
{
    uuid128_t id1{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
    uuid128_t id2{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
    REQUIRE(id1 == id2);
}

TEST_CASE("Uuid not equal operator", "[uuid]")
{
    uuid128_t id1{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uuid128_t id2{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    REQUIRE(id1 != id2);
}

TEST_CASE("Uniqueness of uuid", "[uuid]")
{
    std::vector<uuid128_t> uuids;
    for(size_t i = 0; i < 9999; ++i)
    {
        uuid128_t  newUuid = UuidGenerator::Generate();
        auto it = std::find_if(uuids.begin(), uuids.end(), [&](uuid128_t x) -> bool { return x == newUuid; } );
        REQUIRE(it == uuids.end());
        uuids.push_back(newUuid);
    }
}
