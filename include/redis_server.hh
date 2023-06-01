#ifndef REDIS_SERVER
#define REDIS_SERVER

#include <cstdint>
#include <string>
#include <fmt/format.h>
#include <vector>
#include <unordered_map>
#include <poll.h>

#include "connection.hh"

class RedisServer {
public:
    RedisServer(std::string& _ip, uint16_t _port);

    ~RedisServer();

    void start();
    void stop();

    RedisServer(const RedisServer&) = delete;
    RedisServer& operator=(const RedisServer&) = delete;

private:
    void start_listen();
    void accept_connection();
    void handle_connection(Connection& conn);

    std::string ip;
    uint16_t port;
    int listener_fd = -1;
    std::unordered_map<int, Connection> clients;
    std::vector<pollfd> poll_args;
};

#endif