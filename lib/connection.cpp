#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fmt/format.h>
#include "connection.hh"

Connection::Connection(const std::string& ip, uint16_t port, int _fd): fd(_fd) {
    client = fmt::format("{}:{}", ip, port);
    read_buf.fill('\0');
    write_buf.fill('\0');
}

Connection::~Connection() {
    if (fd != -1) {
        close(fd);
    }
}

inline bool Connection::is_connected() {
    return fd != -1;
}

inline pollfd Connection::get_pfd() {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = state == STATE_REQ ? POLLIN : POLLOUT;
    pfd.events |= POLLERR;
    return pfd;
}

inline uint32_t Connection::get_state() {
    return state;
}

inline bool Connection::operator==(const Connection &other) const noexcept {
    return fd == other.fd;
}

Connection::Connection(Connection&& other) noexcept 
    : fd(other.fd),
        read_buf(std::move(other.read_buf)),
        write_buf(std::move(other.write_buf)) {
    other.fd = -1;
    other.read_buf.fill('\0');
    other.write_buf.fill('\0');
}

Connection& Connection::operator=(Connection &&other) noexcept {
    if (this != &other) {
        if (fd != -1) {
            close(fd);
        }
        fd = other.fd;
        read_buf = std::move(other.read_buf);
        write_buf = std::move(other.write_buf);
        other.fd = -1;
        other.read_buf.fill('\0');
        other.write_buf.fill('\0');
    }
    return *this;
}