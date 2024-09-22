#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include <unistd.h>

void *thread_func(void *arg) {
    for (int i = 0; i < 1; i++) {
        printf("hello from uthread %d\n", *((int *) arg));
        sleep(1);
        yield();
    }
    return NULL;
}

int main() {

    uthread_t uthread1, uthread2, uthread3;
    int thread_num1, thread_num2;
    thread_num1 = 1;
    thread_num2 = 2;
    uthread_create(&uthread1, thread_func, &thread_num1);
    uthread_create(&uthread2, thread_func, &thread_num2);

    for (int i = 0; i < 6; i++) {
        printf("hello from main thread\n");
        sleep(1);
        yield();
    }

    int thread_num3 = 3;
    uthread_create(&uthread3, thread_func, &thread_num3);
    for (int i = 0; i < 4; i++) {
        printf("hello from main thread\n");
        sleep(1);
        yield();
    }

}