#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

void *func1(void *args) {
    static int ret_val = 42;
    return &ret_val;
}

void *func2(void *args) {
    static char *ret_val = "hello world";
    return ret_val;
}

void *func3(void *args) {
    pthread_t thread = pthread_self();
    //pthread_detach(thread);
    printf("идентификатор потока: %lu\n", thread);
    return NULL;
}

int main() {
//// 1 часть
//    pthread_t thread1, thread2;
//    int err = pthread_create(&thread1, NULL, func1, NULL);
//
//    if (err != 0) {
//        perror("thread_create failed");
//        return -1;
//    }
//
//    err = pthread_create(&thread2, NULL, func2, NULL);
//
//    if (err != 0) {
//        perror("thread_create failed");
//        return -1;
//    }
//
//    void *ret_val;
//    pthread_join(thread1, &ret_val);
//    printf("got value from another thread: '%d'\n", *(int *)ret_val);
//    pthread_join(thread2, &ret_val);
//    printf("got string from another thread: '%s'\n", (char *) ret_val);
//    return 0;


//  2 часть

    pthread_t thread;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    while (true) {
        int err = pthread_create(&thread, &attr, func3, NULL);

        if (err != 0) {
            perror("thread create failed");
            break;
        }

        pthread_join(thread, NULL);

    }

    pthread_attr_destroy(&attr);
}