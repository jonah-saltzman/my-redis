#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>

void do_something(int conn_fd);

int main() {
    std::cout << "hello\n";
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        std::cerr << "socket() failed: " << strerror(errno);
        return 1;
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        std::cerr << "bind() failed: " << strerror(errno);
        return 1;
    }
    std::cout << "bound\n";
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        std::cerr << "listen() failed: " << strerror(errno);
        return 1;
    }
    std::cout << "listening...\n";
    while (true) {
        struct sockaddr_in client;
        socklen_t socklen = sizeof(client);
        int client_fd = accept(fd, (sockaddr *)&client, &socklen);
        if (client_fd < 0) {
            std::cerr << "accept() failed: " << strerror(errno);
            continue;
        }
        do_something(client_fd);
        close(client_fd);
    }
    return 0;
}

void do_something(int client_fd) {
    size_t buf_size = 64;
    std::vector<char> buf(buf_size);
    ssize_t n = read(client_fd, buf.data(), buf_size);
    if (n < 0) {
        std::cerr << "read() err" << std::endl;
        return;
    }
    std::string msg(buf.data());
    std::cout << "client says: " << msg << std::endl;
    msg.insert(0, "ECHO: ");
    write(client_fd, msg.c_str(), msg.length());
}