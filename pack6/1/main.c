#include <stdio.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>

#define PAGE_SIZE 4096
const char *shared_file = "shared";

void writer(unsigned int *memory, int num) {
//    int shared_fd = open(shared_file, O_RDWR | O_CREAT, 0660);
//    ftruncate(shared_fd, 0);
//    ftruncate(shared_fd, PAGE_SIZE);
//    unsigned int *memory = mmap(NULL, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shared_fd, 0);
//    close(shared_fd);
//    int num = PAGE_SIZE / sizeof(unsigned int);

    unsigned int curr_num = 0;
    while (true) {
        for (int i = 0; i < num; i++) {
            memory[i] = curr_num++;
            //sleep(1);
        }
    }
    munmap(memory, PAGE_SIZE);

}

void reader(unsigned int *memory, int num) {
    sleep(1);
//    int shared_fd = open(shared_file, O_RDWR | O_CREAT, 0660);
//    unsigned int *memory = mmap(NULL, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shared_fd, 0);
//    close(shared_fd);
//    int num = PAGE_SIZE / sizeof(unsigned int);

    unsigned int prev_read = memory[0];
    unsigned int curr_read;

    while (true) {
        for (int i = 0; i < num; i++) {
            curr_read = memory[i];
            if (prev_read > curr_read) {
                printf("failure occurred: %u %u\n", prev_read, curr_read);
            }
            //sleep(1);
            fflush(stdout);
            prev_read = curr_read;
        }
    }
    munmap(memory, PAGE_SIZE);
}

int main() {
    unsigned int *memory = mmap(NULL, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int num = PAGE_SIZE / sizeof(unsigned int);
    int pid = fork();


    if (pid < 0) {
        return -1;
    }

    if (pid > 0) {
        writer(memory, num);
    } else {
        reader(memory, num);
    }
}