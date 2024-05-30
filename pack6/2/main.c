#include <stdio.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define PAGE_SIZE 4096

void writer(int fd) {
    int num = PAGE_SIZE / sizeof(unsigned int);

    unsigned int curr_num = 0;
    while (true) {
        for (int i = 0; i < num; i++) {
            write(fd, &curr_num, sizeof(unsigned int));
            curr_num++;
            //sleep(1);
        }
    }

}

void reader(int fd) {
    sleep(1);
    int num = PAGE_SIZE / sizeof(unsigned int);
    unsigned int prev_read = 0;
    unsigned int curr_read;

    while (true) {
        for (int i = 0; i < num; i++) {
            read(fd, &curr_read, sizeof(unsigned int));
            if (prev_read > curr_read) {
                printf("failure occurred: %u %u\n", prev_read, curr_read);
                fflush(stdout);
            }
            //sleep(1);
            prev_read = curr_read;
        }
    }
}

int main() {
    int pipes[2];
    pipe(pipes);
    int pid = fork();

    if (pid < 0) {
        return -1;
    }

    if (pid > 0) {
        writer(pipes[1]);
    } else {
        reader(pipes[0]);
    }
    close(pipes[0]);
    close(pipes[1]);
}