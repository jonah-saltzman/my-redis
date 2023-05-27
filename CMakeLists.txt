cmake_minimum_required(VERSION 3.0)
project(my-redis)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Wextra)
add_compile_options(-std=c++20)

find_package(fmt CONFIG REQUIRED)

add_library(core lib/stream_io.cpp)
target_link_libraries(core PUBLIC fmt::fmt)
target_include_directories(core PUBLIC include)

add_library(server lib/redis_server.cpp)
add_library(client lib/redis_client.cpp)
target_link_libraries(server PRIVATE core)
target_link_libraries(client PRIVATE core)

add_executable(redis_server src/server.cpp)
add_executable(redis_client src/client.cpp)
target_include_directories(redis_server PUBLIC include)
target_include_directories(redis_client PUBLIC include)
target_link_libraries(redis_server PUBLIC server fmt::fmt)
target_link_libraries(redis_client PUBLIC client fmt::fmt)