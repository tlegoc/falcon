#include <iostream>
#include <spdlog/spdlog.h>

#include <library.h>

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    hello();
    return EXIT_SUCCESS;
}