cmake_minimum_required(VERSION 3.22)
project(proxy)

set(CMAKE_C_STANDARD 17)
add_executable(server server.c request.h request.c hashmap/hashmap.c hashmap/hashmap.h channel/channel.c channel/channel.h
        logger/logger.c
        logger/logger.h)
add_executable(client client.c)
#add_executable(hashmap hashmap/hashmap.c hashmap/hashmap.h hashmap/test.c)