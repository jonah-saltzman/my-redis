#include <cstdint>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fmt/format.h>
#include "redis_server.hh"

int main(int argc, char* argv[]) {
    try {
        std::uint16_t port = argc == 2 ? std::stoi(argv[1]) : 8080;
        std::string ip("127.0.0.1");
        RedisServer s(ip, port);
        s.start();
    } catch (const std::exception& e) {
        std::cerr << fmt::format("Exception: {}\n", e.what());
    } catch (...) {
        std::cerr << "Unknown exception\n";
    }
}