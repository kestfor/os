cmake_minimum_required(VERSION 3.22)
project(pack2)

SET(CMAKE_C_STANDARD 17)

add_compile_options("-lthread")

add_executable(mutex mutex/main.c custom_mutex/list.c custom_mutex/list.h)
add_executable(spinlock spinlock/main.c spinlock/list.c spinlock/list.h)
add_executable(rwlock rwlock/main.c rwlock/list.c rwlock/list.h)
add_executable(custom_mutex custom_mutex/main.c custom_mutex/mutex.h custom_mutex/mutex.c custom_mutex/list.c custom_mutex/list.h)
add_executable(custom_spinlock custom_spinlock/main.c custom_spinlock/spinlock.c custom_spinlock/spinlock.h custom_spinlock/list.h custom_spinlock/list.c)