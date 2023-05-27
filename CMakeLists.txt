cmake_minimum_required(VERSION 3.0)
project(my-redis)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Wextra)
add_compile_options(-std=c++20)

add_executable(redis_server src/redis_server.cpp src/stream_io.cpp)
add_executable(redis_client src/redis_client.cpp src/stream_io.cpp)
include_directories(include)

find_package(fmt CONFIG REQUIRED)

target_link_libraries(redis_server PRIVATE fmt::fmt)
target_link_libraries(redis_client PRIVATE fmt::fmt)
