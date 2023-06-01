#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <cstring>
#include <fmt/format.h>
#include <poll.h>

#include "stream_io.hh"
#include "connection.hh"
#include "redis_server.hh"

void do_something(int conn_fd);

RedisServer::RedisServer(std::string& _ip, uint16_t _port): ip(_ip), port(_port) {}

RedisServer::~RedisServer() {
    if (listener_fd != -1) {
        close(listener_fd);
    }
}

void RedisServer::start() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        throw std::runtime_error(fmt::format("failed to create socket: {}", strerror(errno)));
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    addr.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        throw std::runtime_error(fmt::format("failed to bind to {}:{}: {}", addr.sin_addr.s_addr, addr.sin_port, strerror(errno)));
    }
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        throw std::runtime_error(fmt::format("listen failed: {}", strerror(errno)));
    }
    fd_set_nb(fd);
    std::cout << fmt::format("listening on port {}\n", port);
}

void RedisServer::start_listen() {
    while (true) {
        poll_args.clear();
        pollfd pfd = {listener_fd, POLLIN, 0};
        poll_args.push_back(pfd);

        for (std::pair<const int, Connection>& pair : conns) {
            if (!pair.second.is_connected()) {
                continue;
            }
            pollfd pfd = pair.second.get_pfd();
            poll_args.push_back(pfd);
        }

        int n = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);

        if (n < 0) {
            throw std::runtime_error(fmt::format("poll err: {}", strerror(errno)));
        }

        for (size_t i = 1; i < poll_args.size(); ++i) {
            if (poll_args[i].revents) {
                Connection& conn = conns[poll_args[i].fd];
                // do some IO
                if (conn.get_state() == Connection::STATE_END) {
                    conns.erase(poll_args[i].fd);
                }
            }
        }

        if (poll_args[0].revents) {
            accept_connection();
        }
    }
}

void RedisServer::accept_connection() {
    struct sockaddr_in client;
    socklen_t socklen = sizeof(client);
    int client_fd = accept(listener_fd, (sockaddr *)&client, &socklen);
    if (client_fd < 0) {
        std::cerr << fmt::format("failed to accept connection: {}\n", strerror(errno));
        return;
    }
    fd_set_nb(client_fd);
    std::string ip(inet_ntoa(client.sin_addr));
    uint16_t port = ntohs(client.sin_port);
    Connection conn(ip, port, client_fd);
    auto inserted = conns.insert({client_fd, std::move(conn)});
    if (!inserted.second) {
        throw std::runtime_error("duplicate FDs in accept_connection()");
    }
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