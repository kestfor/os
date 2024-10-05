cmake_minimum_required(VERSION 3.22)
project(pack2)

SET(CMAKE_C_STANDARD 17)


#add_executable(queue2 queue.h queue-example.c queue.c queue-threads.c)
#add_executable(queue_spinlock spinlock/queue.h queue-threads.c spinlock/queue.c)
#add_executable(queue_mutex mutex/queue.h queue-threads.c mutex/queue.c)
#add_executable(queue_cond cond_val/queue.h queue-threads.c cond_val/queue.c)
add_executable(queue_sem sem/queue.h queue-threads.c sem/queue.c)