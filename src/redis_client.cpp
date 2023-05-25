#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("socket() failed: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        printf("connect() failed: %s\n", strerror(errno));
        return 1;
    }

    char msg[64];
    while (fgets(msg, sizeof(msg), stdin) != NULL) {
        write(fd, msg, strlen(msg));

        char rbuf[64] = {};
        ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
        if (n < 0) {
            printf("read() failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        printf("%s\n", rbuf);
    }
    close(fd);
    return 0;
}
