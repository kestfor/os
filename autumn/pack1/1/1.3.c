#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct func_args {
    int int_arg;
    char *charpt_arg;
} func_args;

void *func(void *args) {
    sleep(1);
    func_args parsed = *(func_args *) args;
    printf("values from args: '%d', '%s'\n", parsed.int_arg, parsed.charpt_arg);
    return NULL;
}

int main() {
    pthread_t thread;
    func_args args = {1, "hello world"};

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int err = pthread_create(&thread, &attr, func, (void *) &args);
    if (err != 0) {
        perror("thread_create failed");
        return -1;
    }

    pthread_attr_destroy(&attr);
    sleep(2);
    pthread_join(thread, NULL);

}