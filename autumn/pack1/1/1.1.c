#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int global_var;

void *mythread(void *arg) {
    static int static_local_var;
    int local_var;
    printf("mythread [%d %d %d]: Hello from mythread!, static local: %p, local: %p, global: %p, pid: %lu\n", getpid(), getppid(),
           gettid(), &static_local_var, &local_var, &global_var, pthread_self());
    sleep(50);
    return NULL;
}

int main() {
    int len = 1;
    pthread_t tids[len];
    int err;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    for (int i = 0; i < len; i++) {
        printf("before\n");
        fflush(stdout);
        err = pthread_create(&tids[i], NULL, mythread, NULL);
        printf("after\n");
        fflush(stdout);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return -1;
        }
    }

    void *ret_val;

    for (int i = 0; i < len; i++) {
        pthread_join(tids[i], &ret_val);
    }

    return 0;
}

