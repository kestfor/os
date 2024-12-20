#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

void sig_handler(int sig) {
    printf("Signal handling thread got SIGINT\n");
    pthread_exit(NULL);
}

void *func1(void *args) {
    sigset_t set;

    sigfillset(&set);
    sigdelset(&set, SIGINT);

    pthread_sigmask(SIG_SETMASK, &set, NULL);

    signal(SIGINT, sig_handler);
    while (true) {
        sleep(1);
    }

    pthread_exit(NULL);
}

void *func2(void *args) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    int sig;

    if (sigwait(&set, &sig) != 0) {
        perror("sigwait failed");
    } else {
        printf("Signal handling thread got SIGQUIT\n");
    }

    pthread_exit(NULL);
}

int
main(int argc, char *argv[]) {
    pthread_t thread1, thread2;
    sigset_t set;
    int s;

    sigfillset(&set);

    s = pthread_sigmask(SIG_SETMASK, &set, NULL);

    if (s != 0) {
        perror("sigmask failed");
        return -1;
    }

    if (pthread_create(&thread1, NULL, &func1, NULL) != 0) {
        perror("pthread create failed");
        return -1;
    }

    if (pthread_create(&thread2, NULL, &func2, NULL) != 0) {
        perror("pthread create failed");
        return -1;
    }

    int pid = getpid();

    sleep(2);
    kill(pid, SIGINT);
    sleep(1);
    kill(pid, SIGQUIT);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

}