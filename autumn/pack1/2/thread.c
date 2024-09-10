#define _GNU_SOURCE

#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include "thread.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define PAGE_SIZE  4096
#define STACK_SIZE (PAGE_SIZE*8)

struct thread {
    start_routine_t start_routine;
    void *args;
    void *result;
    volatile int exited;
    volatile int joined;
};

void *create_stack(int stack_size) {
//    char stack_file[128];
//    int stack_fd;
    void *stack;
//    snprintf(stack_file, sizeof(stack_file), "stack-%d", id);
//    stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
//    ftruncate(stack_fd, 0);
//    ftruncate(stack_fd, stack_size);

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
    while (!th->joined) {
        sleep(1);
    }
    return 0;
}

int thread_create(thread_t *th, start_routine_t start_routine, void *args) {
    void *stack = create_stack(STACK_SIZE);
    mprotect(stack + PAGE_SIZE, STACK_SIZE - PAGE_SIZE, PROT_READ | PROT_WRITE);
    memset(stack + PAGE_SIZE, 0, STACK_SIZE - PAGE_SIZE);

    thread_struct *new_thread = (stack + STACK_SIZE - sizeof(thread_struct));
    new_thread->start_routine = start_routine;
    new_thread->args = args;
    new_thread->result = NULL;
    new_thread->exited = false;
    new_thread->joined = false;

    stack = (void *) new_thread;

    int err = clone(start_routine_wrapper,
                    stack,
                    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SIGHAND | SIGCHLD |
                    CLONE_SYSVSEM | CLONE_CHILD_CLEARTID | CLONE_PARENT_SETTID,
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
    while (!thread->exited) {
        sleep(1);
    }
    if (result != NULL) {
        *result = thread->result;
    }
    thread->joined = true;
    return 0;
}