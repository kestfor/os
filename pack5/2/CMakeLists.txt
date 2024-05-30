cmake_minimum_required(VERSION 3.22)
project(pack5)

SET(CMAKE_C_STANDARD 17)
add_executable(child_zombie child_zombie.c)
add_executable(parent_zombie parent_zombie.c)