#ifndef REDIS_CONN
#define REDIS_CONN

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fmt/format.h>
#include <poll.h>

class Connection {
public:
    enum {
        STATE_REQ = 0,
        STATE_RES = 1,
        STATE_END = 2
    };
    std::string client;

    Connection(const std::string& ip, uint16_t port, int _fd);

    ~Connection();

    bool is_connected();
    pollfd get_pfd();
    uint32_t get_state();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection) = delete;

    bool operator==(const Connection& other) const noexcept;
    Connection(Connection&& other) noexcept;

    Connection& operator=(Connection&& other) noexcept;

private:
    static constexpr size_t BUF_SIZE = 4096;
    int fd = -1;
    std::array<char, BUF_SIZE> read_buf;
    std::array<char, BUF_SIZE> write_buf;
    uint32_t state = 0;
};

#endif