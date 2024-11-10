#define _GNU_SOURCE
#include <stdio.h>
#include "mutex.h"
#include <pthread.h>

mutex m;
int counter_var = 0;

void *counter() {
    int tid = gettid();
    printf("tid: %d\n", tid);
    for (int i = 0; i < 10000; i++) {
        int err = lock_mutex(&m);
        if (err != 0) {
            printf("tid %d, after lock, err: %d, counter: %d, owner: %d\n", tid, err, m.counter, m.owner);
        }
        fflush(stdout);
        counter_var++;
        err = unlock_mutex(&m);
        printf("tid %d, after unlock, err: %d, counter: %d, owner: %d\n", tid, err, m.counter, m.owner);
        fflush(stdout);
    }
    return NULL;
}

void *func1() {
    int err = lock_mutex(&m);
    err = lock_mutex(&m);
    printf("%d\n", err);
    return NULL;
}

void *func2() {
    sleep(1);
    int err = unlock_mutex(&m);
    printf("%d\n", err);
    return NULL;
}

int main() {
    mutex_init(&m, RECURSIVE);
    int err;
    err = lock_mutex(&m);
    printf("err: %d\n", err);
    err = lock_mutex(&m);
    printf("err: %d\n", err);
    err = lock_mutex(&m);
    printf("err: %d\n", err);


//    pthread_t first, second;
//    pthread_create(&first, NULL, func1, NULL);
//    pthread_create(&second, NULL, func2, NULL);
//
//    pthread_join(first, NULL);
//    pthread_join(second, NULL);
//    printf("%d", counter_var);
}
