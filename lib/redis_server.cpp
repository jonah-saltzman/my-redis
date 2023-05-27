#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <cstring>
#include "stream_io.hh"
#include <fmt/format.h>

void do_something(int conn_fd);

int start_server(std::uint32_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        std::cerr << "socket() failed: " << strerror(errno);
        return 1;
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    addr.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        return 1;
    }
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        std::cerr << "listen() failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << fmt::format("listening on port {}\n", port);
    while (true) {
        struct sockaddr_in client;
        socklen_t socklen = sizeof(client);
        int client_fd = accept(fd, (sockaddr *)&client, &socklen);
        if (client_fd < 0) {
            std::cerr << "accept() failed: " << strerror(errno);
            continue;
        }
        while (true) {
            try {
                do_something(client_fd);
            } catch (std::runtime_error& e) {
                std::cerr << fmt::format("do_something: {}\n", e.what());
                break;
            }
        }
        close(client_fd);
    }
    return 0;
}

void do_something(int client_fd) {
    // get length header
    std::uint32_t len;
    std::vector<char> buf = read_bytes(client_fd, sizeof(len));
    memcpy(&len, buf.data(), sizeof(len));
    //std::cerr << fmt::format("do_something recv msg len {}\n", len);
    // read payload
    buf = read_bytes(client_fd, len);
    //std::cerr << fmt::format("msg: {}\n", buf.data());
    // compose reply
    std::string_view recv(buf.data(), buf.size());
    std::string reply("ECHO: ");
    reply.append(recv.begin(), recv.end());

    // send reply header
    buf = std::vector<char>(sizeof(len));
    len = reply.length();
    memcpy(buf.data(), &len, sizeof(len));
    //std::cerr << fmt::format("sending reply header={}\n", *((std::uint32_t*)buf.data()));
    write_bytes(client_fd, buf);

    // send reply
    //std::cerr << fmt::format("sending reply (len={}): {}\n", reply.length(), reply.data());
    std::vector<char> reply_vec = std::vector<char>(reply.begin(), reply.end());
    write_bytes(client_fd, reply_vec);
}