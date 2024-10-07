cmake_minimum_required(VERSION 3.22)
project(pack2)

SET(CMAKE_C_STANDARD 17)

add_compile_options("-lthread")

add_executable(mutex mutex/main.c mutex/list.c mutex/list.h)
add_executable(spinlock spinlock/main.c spinlock/list.c spinlock/list.h)
add_executable(rwlock rwlock/main.c rwlock/list.c rwlock/list.h)

