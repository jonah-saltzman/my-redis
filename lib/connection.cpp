#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fmt/format.h>
#include <cassert>
#include <iostream>
#include <string>
#include "connection.hh"
#include "redis_server.hh"

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

void Connection::state_req() {
    while (try_fill_buf()) {}
}

void Connection::state_res() {
    while (try_flush_buf()) {}
}

bool Connection::try_fill_buf() {
    assert(read_pos < BUF_SIZE);
    ssize_t r = 0;
    do {
        size_t capacity = BUF_SIZE - read_pos;
        r = read(fd, read_buf.data() + read_pos, capacity);
    } while (r < 0 && errno == EINTR);
    if (r < 0 && errno == EAGAIN) {
        return false;
    }
    if (r < 0) {
        std::cerr << fmt::format("{}: error reading from stream: {}\n", client, strerror(errno));
        state = STATE_END;
        return false;
    }
    if (r == 0) {
        if (read_pos) {
            std::cerr << fmt::format("{}: unexpected EOF\n", client);
        }
        state = STATE_END;
        return false;
    }
    read_pos += r;
    assert(read_pos < BUF_SIZE);
    while (try_one_request()) {}
    return state == STATE_REQ;
}

bool Connection::try_flush_buf() {
    assert(write_pos < BUF_SIZE);

    ssize_t written = 0;
    do {
        size_t to_write = write_pos - write_sent;
        written = write(fd, write_buf.data() + write_pos, to_write);
    } while (written < 0 && errno == EINTR);

    if (written < 0 && errno == EAGAIN) {
        return false;
    }

    if (written < 0) {
        std::cerr << fmt::format("{}: write error: {}\n", client, strerror(errno));
        state = STATE_END;
        return false;
    }

    if (written == 0) {
        std::cerr << fmt::format("{}: unexpected EOF\n", client);
        state = STATE_END;
        return false;
    }

    write_sent += written;

    if (write_sent == write_pos) {
        state = STATE_REQ;
        write_pos = 0;
        write_sent = 0;
        return false;
    }

    return true;
}

bool Connection::try_one_request() {
    // get length header
    if (read_pos < HEADER_LEN) {
        return false; // not enough data
    }

    uint32_t len;
    memcpy(&len, read_buf.data(), HEADER_LEN);

    if (len > HEADER_LEN + BUF_SIZE) {
        std::cerr << fmt::format("{}: msg len {} too long\n", client, len);
        state = STATE_END;
        return false;
    }

    if (len == 0) {
        std::cerr << fmt::format("{}: disconnected\n", client);
        state = STATE_END;
        return false;
    }

    if (len + HEADER_LEN > + read_pos) {
        // not enough data
        return false;
    }

    len += 6;
    if (len + HEADER_LEN > BUF_SIZE) {
        std::cerr << fmt::format("{}: response was too long, disconnecting\n", client);
        state = STATE_END;
        return false;
    }
    
    // copy the header
    memcpy(write_buf.data(), &len, HEADER_LEN);

    // copy `ECHO: `
    memcpy(write_buf.data() + HEADER_LEN, echo.data(), echo.size());

    // copy the message
    memcpy(write_buf.data() + HEADER_LEN + echo.size(), read_buf.data() + HEADER_LEN, len);

    write_pos = write_pos + HEADER_LEN + echo.size() + len;

    size_t remaining = read_pos - (HEADER_LEN + len);

    if (remaining) {
        memmove(read_buf.data(), read_buf.data() + HEADER_LEN + len, remaining);
    }

    state = STATE_RES;
    state_res();
    return state == STATE_REQ;
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