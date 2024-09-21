#define _GNU_SOURCE

#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include "thread.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define PAGE_SIZE  4096
#define STACK_SIZE (PAGE_SIZE*8)

struct thread {
    start_routine_t start_routine;
    void *args;
    void *result;
    enum THREAD_TYPE type;
    volatile int exited;
};

void *create_stack(int stack_size) {
    void *stack;
    stack = mmap(NULL, stack_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return stack;
}

int clean_up(thread_struct *th) {
    return munmap(th + sizeof(thread_struct) - STACK_SIZE, STACK_SIZE);
}

int start_routine_wrapper(void *args) {
    thread_struct *th = args;
    void *result = th->start_routine(th->args);
    th->result = result;
    th->exited = true;
    if (th->type == DETACHED) {
        return clean_up(th);
    }
    return 0;
}

int thread_create(thread_t *th, start_routine_t start_routine, void *args, enum THREAD_TYPE type) {
    void *stack = create_stack(STACK_SIZE);
    if (stack == (void *)-1) {
        return -1;
    }

    if (mprotect(stack + PAGE_SIZE, STACK_SIZE - PAGE_SIZE, PROT_READ | PROT_WRITE) != 0) {
         return -1;
    }
    memset(stack + PAGE_SIZE, 0, STACK_SIZE - PAGE_SIZE);

    thread_struct *new_thread = (stack + STACK_SIZE - sizeof(thread_struct));
    new_thread->start_routine = start_routine;
    new_thread->args = args;
    new_thread->result = NULL;
    new_thread->exited = false;
    new_thread->type = type;

    stack = (void *) new_thread;

    int err = clone(start_routine_wrapper,
                    stack,
                    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SIGHAND | SIGCHLD |
                    CLONE_SYSVSEM,
                    new_thread);


    if (err == -1) {
        perror("clone failed");
        exit(-1);
    }

    *th = new_thread;
    return 0;
}

int thread_join(thread_t th, void **result) {
    thread_struct *thread = th;

    if (th->type == DETACHED) {
        return -1;
    }

    while (!thread->exited) {
        usleep(10000);
    }
    if (result != NULL) {
        *result = thread->result;
    }
    return clean_up(th);
}