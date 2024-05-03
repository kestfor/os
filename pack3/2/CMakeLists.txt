cmake_minimum_required(VERSION 3.22)
project(pack3)

set(CMAKE_C_STANDARD 17)
add_executable(main main.c implementations.c func_decl.h func_types.h)
