#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

//int counter = 0;

void *func(void *args) {
    while (true) {
        printf("hello world\n");
        fflush(stdout);
    }
    pthread_exit(NULL);
}

void *func2(void *args) {
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int counter;
    while (true) {
        counter++;
    }
    pthread_exit(NULL);
}



typedef struct arguments {
    char *buff;
} arguments;

void clean_up_routine(void *args) {
    arguments *parsed = (arguments *)args;
    free(parsed->buff);
    parsed->buff = NULL;
    printf("clean up routine...");
    fflush(stdout);
}

void *func3(void *args) {
    char *buff;
    pthread_cleanup_push(clean_up_routine, &buff)
    buff = malloc(sizeof(char) * 12);
    memcpy(buff, "hello world\0", 12 * sizeof(char));
    while (true) {
        pthread_testcancel();
        printf("%s\n", buff);
    }
    pthread_cleanup_pop(1);
    return NULL;
}


int main() {
    pthread_t thread;

//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int err = pthread_create(&thread, NULL, func2, NULL);
    if (err != 0) {
        perror("thread_create failed");
        return -1;
    }

    pthread_cancel(thread);
    pthread_exit(NULL);

}