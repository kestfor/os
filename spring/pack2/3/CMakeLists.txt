cmake_minimum_required(VERSION 3.22)
project(pack3)

SET(CMAKE_C_STANDARD 17)
add_executable(my_strace.o my_strace.c syscallent.h)