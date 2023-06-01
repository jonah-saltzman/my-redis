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
#include <fcntl.h>
#include <cerrno>
#include <iostream>
#include <cstring>
#include <fmt/format.h>
#include <poll.h>

#include "redis_server.hh"

void fd_set_nb(int fd);

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
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);
    int r = inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    if (r != 1) {
        throw std::runtime_error(fmt::format("invalid ip: {}", ip));
    }
    r = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (r) {
        throw std::runtime_error(fmt::format("failed to bind to {}:{}: {}", addr.sin_addr.s_addr, addr.sin_port, strerror(errno)));
    }
    r = listen(fd, SOMAXCONN);
    if (r) {
        throw std::runtime_error(fmt::format("listen failed: {}", strerror(errno)));
    }
    fd_set_nb(fd);
    listener_fd = fd;
    std::cout << fmt::format("listening on port {}\n", port);
    start_listen();
}

void RedisServer::start_listen() {
    while (true) {
        poll_args.clear();
        pollfd pfd = {listener_fd, POLLIN, 0};
        poll_args.push_back(pfd);

        for (std::pair<const int, Connection>& pair : clients) {
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
                Connection& conn = clients.at(poll_args[i].fd);
                handle_connection(conn);
                if (conn.get_state() == Connection::STATE_END) {
                    clients.erase(poll_args[i].fd);
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
        throw std::runtime_error(fmt::format("failed to accept connection: {}\n", strerror(errno)));
    }
    fd_set_nb(client_fd);
    std::string ip(inet_ntoa(client.sin_addr));
    uint16_t port = ntohs(client.sin_port);
    Connection conn(ip, port, client_fd);
    std::cerr << fmt::format("accepted {}\n", conn.client);
    auto inserted = clients.insert({client_fd, std::move(conn)});
    if (!inserted.second) {
        throw std::runtime_error("duplicate FDs in accept_connection()");
    }
}

void RedisServer::handle_connection(Connection& conn) {
    switch (conn.get_state()) {
    case Connection::STATE_REQ:
        conn.state_req();
        break;
    case Connection::STATE_RES:
        conn.state_res();
        break;
    case Connection::STATE_END:
        clients.erase(conn.get_fd());
        break;
    }
}

void fd_set_nb(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno != 0) {
        throw std::runtime_error(fmt::format("fcntl failed: {}", strerror(errno)));
    }
    flags |= O_NONBLOCK;
    errno = 0;
    (void)fcntl(fd, F_SETFL, flags);
    if (errno != 0) {
        throw std::runtime_error(fmt::format("fcntl failed: {}", strerror(errno)));
    }
}
