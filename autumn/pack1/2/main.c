#define _GNU_SOURCE

#include "thread.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
static int a = 42;

void *func(void *args) {
    write(1, "hello world\n", 12);
    return (void *) &a;
}

int main() {
    int len = 5;
    thread_t threads[len];
    int args[len];

    for (int i = 0; i < len; i++) {
        args[i] = i;
        int err = thread_create(&threads[i], func, &args[i], JOINABLE);
        if (err != 0) {
            printf("thread create failed");
        }
    }

    for (int i = 0; i < len; i++) {
        thread_join(threads[i], NULL);
    }
}