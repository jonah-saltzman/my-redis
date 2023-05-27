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

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // replace with server's IP if not localhost
    int rv = connect(fd, (const sockaddr*)&addr, sizeof(addr));
    if (rv < 0) {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        return 1;
    }
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
