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

class RedisServer;

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

    bool is_connected() {
        return fd != -1;
    }

    pollfd get_pfd() {
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = state == STATE_REQ ? POLLIN : POLLOUT;
        pfd.events |= POLLERR;
        return pfd;
    }

    int get_fd() {
        return fd;
    }

    uint32_t get_state() {
        return state;
    }

    bool try_fill_buf();
    bool try_flush_buf();
    void state_req();
    void state_res();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection) = delete;

    bool operator==(const Connection& other) const noexcept;
    Connection(Connection&& other) noexcept;

    Connection& operator=(Connection&& other) noexcept;

private:
    bool try_one_request();

    static constexpr size_t BUF_SIZE = 4096;
    static constexpr size_t HEADER_LEN = sizeof(uint32_t);

    static constexpr char echo_str[] = "ECHO: ";
    static constexpr std::array<char, sizeof(echo_str) - 1> echo = { 'E', 'C', 'H', 'O', ':', ' ' };

    int fd = -1;
    std::array<char, BUF_SIZE> read_buf;
    std::array<char, BUF_SIZE> write_buf;
    size_t read_pos = 0;
    size_t write_pos = 0;
    size_t write_sent = 0;
    uint32_t state = 0;
};

#endif