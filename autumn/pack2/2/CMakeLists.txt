cmake_minimum_required(VERSION 3.22)
project(pack2)

SET(CMAKE_C_STANDARD 17)

add_compile_options("-lthread")

add_executable(main.o main.c list.c list.h)
