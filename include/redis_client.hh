#ifndef REDIS_CLIENT
#define REDIS_CLIENT

#include <cstdint>
#include <string>

int start_client(std::uint32_t port, std::string ip);

#endif