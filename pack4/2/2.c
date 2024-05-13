#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

void cycle() {
    int size = 100;
    int *mem[size];
    for (int i = 0; i < size; i++) {
        mem[i] = malloc(1024 * 1024);
        sleep(1);
    }
    for (int i = 0; i < size; i++) {
        free(mem[i]);
    }
}

void recursion() {
    char buff[PAGE_SIZE];
    memset(buff, 0, PAGE_SIZE);
    sleep(1);
    recursion();
}

void sig_handler(int sig) {
    perror("received SIGSEGV");
    sleep(3);
    exit(1);
//    if (sig == SIGSEGV) {
//        printf("received SIGSEGV\n");
//    } else {
//        printf("received not SIGSEGV\n");
//    }
}

int main(int argc, char *argv[]) {
    printf("pid: %d\n", getpid());
    signal(SIGSEGV, sig_handler);
    sleep(10);
    //recursion();
    //cycle();
    int *p = mmap(NULL, PAGE_SIZE * 10, PROT_NONE, MAP_ANONYMOUS, -1, 0);
    printf("memory map 10 pages\n");
    sleep(3);
    mprotect(p, PAGE_SIZE * 10, PROT_READ);
    printf("memory protect\n");
    //int a = p[0];
    p[0] = 1;
    sleep(3);
    munmap(p + PAGE_SIZE * 4, PAGE_SIZE * 2);
    printf("memory unmap from 4 to 6 page");
    sleep(3);
}