#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdint>
#include "stream_io.hh"
#include <fmt/format.h>

int start_client(std::uint32_t port, std::string ip) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return 1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    int r = inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    if (r != 1) {
        throw std::runtime_error(fmt::format("invalid ip: {}", ip));
    }
    int rv = connect(fd, (const sockaddr*)&addr, sizeof(addr));
    if (rv < 0) {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "client connected\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            // send message size
            std::uint32_t len = line.length();
            std::vector<char> buf(sizeof(len));
            memcpy(buf.data(), &len, sizeof(len));
            write_bytes(fd, buf);

            // send message
            std::vector<char> msg(line.begin(), line.end());
            write_bytes(fd, msg);

            // read reply size
            buf = read_bytes(fd, sizeof(len));
            memcpy(&len, buf.data(), sizeof(len));

            // read reply
            buf = read_bytes(fd, len);

            std::string reply(buf.begin(), buf.end());
            std::cout << reply << std::endl;
        } catch (std::runtime_error& e) {
            std::cerr << fmt::format("{}\ndisconnecting\n", e.what());
            break;
        }
    }
    close(fd);
    return 0;
}
