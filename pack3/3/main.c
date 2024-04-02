#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE 4096
#define BUFF_SIZE 256

int inspect_file(int fd, uint64_t start_addr, uint64_t end_addr) {
    for (uint64_t i = start_addr; i <= end_addr; i += PAGE_SIZE) {
        uint64_t data;
        uint64_t index = (i / PAGE_SIZE) * sizeof(uint64_t);
        if (pread(fd, &data, sizeof(data), (long) index) != sizeof(data)) {
            perror("error while reading file");
            return -1;
        }
        printf("addr: 0x%lx, "
               "page frame num: %lx, "
               "swap type: %ld, "
               "swap offset: %lx, "
               "soft-dirty: %ld, "
               "exclusively mapped: %ld, "
               "file-page/shared-anon: %ld, "
               "swapped: %ld, "
               "present: %ld\n",
               i, data & 0x7fffffffffffff, data & 15, data & (0x7fffffffffffff - 15), (data >> 55) & 1, (data >> 56) & 1,
               (data >> 61) & 1, (data >> 62) & 1, (data >> 63) & 1
        );
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: %s pid, start addr, end addr\nor\n%s pid, addr\n     ", argv[0], argv[0]);
        return 0;
    }

    int errno = 0;
    int pid = (int) strtol(argv[1], NULL, 10);
    uint64_t start_addr = strtol(argv[2], NULL, 16);
    uint64_t end_addr = argc == 3 ? start_addr : strtol(argv[3], NULL, 16);

    if (errno != 0) {
        printf("invalid arguments\n");
        return -1;
    }

    char filename[BUFF_SIZE];
    snprintf(filename, BUFF_SIZE, "/proc/%d/pagemap", pid);

    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        perror("Couldn't open pagemap file");
        return -1;
    }

    int res_code = inspect_file(fd, start_addr, end_addr);
    close(fd);
    return res_code;
}