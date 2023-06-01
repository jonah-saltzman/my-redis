#ifndef REDIS_IO
#define REDIS_IO

#include <vector>

std::vector<char> read_bytes(int fd, std::size_t bytes);

void write_bytes(int fd, std::vector<char>& buf);

void fd_set_nb(int fd);

#endif