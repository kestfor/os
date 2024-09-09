#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

//void *func1(void *args) {
//    sigset_t set;
//    sigemptyset(&set);
//    sigaddset(&set, SIGQUIT);
//    sigaddset(&set, SIGINT);
//
//    int s = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
//
//    if (s != 0) {
//        perror("sigmask failed");
//    }
//
//    int sig;
//
//    s = sigwait(&set, &sig);
//    if (s != 0) {
//        perror("sigwait error");
//    }
//    printf("Signal handling thread got signal %d\n", sig);
//
//    pthread_exit(NULL);
//}
//
//void *func2(void *args) {
//    return NULL;
//}
//
//void *func3(void *args) {
//    return NULL;
//}
//
//
//int main() {
//    pthread_t thread1, thread2, thread3;
//    sigset_t set;
//    sigfillset(&set);
//    pthread_sigmask(SIG_SETMASK, &set, NULL);
//
//    int err = pthread_create(&thread1, NULL, func1, NULL);
//    if (err != 0) {
//        perror("pthread failed");
//    }
//    pthread_join(thread1, NULL);
//
//}

void sig_handler(int sig) {
    printf("Signal handling thread got SIGINT\n");
}

void *func1(void *args) {
//    sigset_t full_set;
//    sigfillset(&full_set);
//    sigset_t set;
//    sigemptyset(&set);
//    sigaddset(&set, SIGINT);
//
//    if (pthread_sigmask(SIG_UNBLOCK, &set, &full_set) != 0) {
//        perror("sigmask failed");
//    }

    signal(SIGINT, sig_handler);

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

    /* Block SIGQUIT and SIGUSR1; other threads created by main()
       will inherit a copy of the signal mask. */

    sigfillset(&set);
    sigdelset(&set, SIGINT);

    s = pthread_sigmask(SIG_SETMASK, &set, NULL);

    if (s != 0) {
        perror("sigmask failed");
        return -1;
    }

    s = pthread_create(&thread1, NULL, &func1, NULL);
    if (s != 0) {
        perror("pthread create failed");
        return -1;
    }

    if (pthread_create(&thread2, NULL, &func2, NULL) != 0) {
        perror("pthread create failed");
        return -1;
    }

    sigset_t before = set;
    sigfillset(&set);

    s = pthread_sigmask(SIG_SETMASK, &set, &before);

    if (s != 0) {
        perror("sigmask failed");
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