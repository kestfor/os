#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>

//#define STACK_SIZE (1024*1024)


void recurs(int depth) {
    if (depth >= 10) {
        return;
    }
    char b = (char) (depth + '0');
    char word[] = "hello world";
    recurs(depth + 1);
}

int child_func() {
    recurs(0);
    return 0;
}

int main(int argc, char **argv) {
    const int STACK_SIZE = 1024*1024;
    void* stack = mmap(0, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (!stack) {
        perror("mmap() failed");
        exit(1);
    }

    if (clone(child_func, stack + STACK_SIZE, SIGCHLD, NULL) == -1) {
        perror("clone");
        exit(1);
    }

    int status;
    if (wait(&status) == -1) {
        perror("wait");
        exit(1);
    }

    char filename[] = "stack.bin";
    int file = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (file < 0) {
        perror("open() failed");
        exit(1);
    }

    if (write(file, stack, STACK_SIZE) == -1) {
        perror("write() failed");
    }

    close(file);
    return 0;
}
