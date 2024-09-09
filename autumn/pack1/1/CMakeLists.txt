cmake_minimum_required(VERSION 3.22)
project(pack1)

SET(CMAKE_C_STANDARD 17)

add_compile_options("-lthread")

add_executable(thread1.o 1.1.c)
add_executable(thread2.o 1.2.c)
add_executable(thread3.o 1.3.c)
add_executable(thread4.o 1.4.c)
add_executable(thread5.o 1.5.c)