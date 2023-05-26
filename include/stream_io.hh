#include <stdexcept>
#include <sys/_types/_ssize_t.h>
#include <vector>
#include <format>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

std::vector<char> read_bytes(int fd, std::size_t bytes) {
    std::vector<char> buffer(bytes);
    std::size_t total_bytes_read = 0;

    while (total_bytes_read < bytes) {
        ssize_t bytes_read = read(fd, buffer.data() + total_bytes_read, bytes - total_bytes_read);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::format("failed to read from stream {}", fd));
        } else if (bytes_read == 0) {
            throw std::runtime_error("unexpected eof");
        } else {
            total_bytes_read += bytes_read;
        }
    }

    return buffer;
}

void write_bytes(int fd, std::vector<char>& buf) {
    std::size_t total_bytes_written = 0;

    while (total_bytes_written < buf.size()) {
        ssize_t bytes_written = write(fd, buf.data() + total_bytes_written, buf.size() - total_bytes_written);
        if (bytes_written < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::format("failed to read from stream {}", fd));
        } else {
            total_bytes_written += bytes_written;
        }
    }
}