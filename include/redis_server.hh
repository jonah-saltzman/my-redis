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
    std::string ip;
    uint16_t port;

    RedisServer(std::string& _ip, uint16_t _port);

    ~RedisServer();

    void start();
    void stop();
    void accept_connection();

    RedisServer(const RedisServer&) = delete;
    RedisServer& operator=(const RedisServer&) = delete;


private:
    void start_listen();
    int listener_fd = -1;
    std::unordered_map<int, Connection> conns;
    std::vector<pollfd> poll_args;
};

#endif