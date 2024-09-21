cmake_minimum_required(VERSION 3.22)
project(pack1)

SET(CMAKE_C_STANDARD 17)

add_compile_options("-lthread")

add_executable(main.o main.c uthread.c
        uthread.h)
