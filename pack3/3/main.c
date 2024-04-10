#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define PAGE_SIZE 4096
#define BUFF_SIZE 256

typedef struct page {
    uint64_t start_addr;
    uint64_t end_addr;
} page;

typedef struct page_array {
    page *array;
    int size;
} page_array;

void clear_page_array(page_array *pg_array) {
    if (pg_array->array != NULL) {
        free(pg_array->array);
    }
}

int get_mem_range(char *maps_filename, page_array *pg_array) {
    int capacity = 128;
    int curr_size = 0;

    errno = 0;
    pg_array->array = malloc(sizeof(page) * capacity);

    if (pg_array->array == NULL) {
        perror("can't allocate enough memory");
        return -1;
    }

    int buff_size = 512;
    char buff[buff_size];
    FILE *file = fopen(maps_filename, "r");

    if (file == NULL) {
        perror("Couldn't open maps file");
        return -1;
    }

    while (fgets(buff, buff_size, file) != NULL) {
        char *space_pt = strchr(buff, ' ');
        if (space_pt == NULL) {
            return -1;
        }
        char *del_pt = strchr(buff, '-');
        if (del_pt == NULL) {
            return -1;
        }

        buff[del_pt - &buff[0]] = '\000';
        buff[space_pt - &buff[0]] = '\000';
        uint64_t start_addr = strtol(buff, NULL, 16);
        uint64_t end_addr = strtol(del_pt + 1, NULL, 16);

        if (errno != 0) {
            perror("Couldn't parse maps file");
            return  -1;
        }

        page new_page = {start_addr, end_addr};
        if (capacity <= curr_size) {
            capacity *= 2;
            page *tmp = realloc(pg_array->array, capacity * sizeof(page));
            if (tmp == NULL) {
                perror("can't allocate enough memory");
                return -1;
            }
            pg_array->array = tmp;
        }

        pg_array->array[curr_size] = new_page;
        curr_size++;

    }
    fclose(file);
    pg_array->size = curr_size;
    return 0;
}

int inspect_file(int fd_from, int fd_to, uint64_t start_addr, uint64_t end_addr) {
    size_t buff_size = 256;
    char buff[buff_size];
    size_t offset = 0;

    for (uint64_t i = start_addr; i <= end_addr; i += PAGE_SIZE) {
        memset(buff, '\0', buff_size);
        uint64_t data;
        uint64_t index = (i / PAGE_SIZE) * sizeof(uint64_t);
        if (pread(fd_from, &data, sizeof(data), (long) index) != sizeof(data)) {
            perror("error while reading file");
            return -1;
        }
        sprintf(buff, "addr: 0x%lx, "
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
        buff_size = strlen(buff);
        if (pwrite(fd_to, buff, buff_size, (long) offset) != sizeof(char) * buff_size) {
            perror("error while writing file");
            return -1;
        }
        offset += buff_size;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage:\n%s pid output_file_name\n     ", argv[0]);
        return 0;
    }

    int pid = (int) strtol(argv[1], NULL, 10);
    if (errno != 0) {
        printf("invalid arguments\n");
        return -1;
    }

    char filename[BUFF_SIZE];
    char maps_path[BUFF_SIZE];
    snprintf(filename, BUFF_SIZE, "/proc/%d/pagemap", pid);
    snprintf(maps_path, BUFF_SIZE, "/proc/%d/maps", pid);

    int fd_pagemap = open(filename, O_RDONLY);
    int fd_output = open(argv[2], O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

    if (fd_pagemap < 0) {
        perror("Couldn't open pagemap file");
        return -1;
    }

    if (fd_output == -1) {
        close(fd_output);
        fd_output = open(argv[2], O_WRONLY);
    }

    if (fd_output < 0) {
        perror("Couldn't open output file");
        close(fd_pagemap);
        return -1;
    }

    page_array pg_array;

    int res_code = get_mem_range(maps_path, &pg_array);
    if (res_code != 0) {
        clear_page_array(&pg_array);
        return -1;
    }

    for (int i = 0; i < pg_array.size; i++) {
        res_code = inspect_file(fd_pagemap, fd_output, pg_array.array[i].start_addr, pg_array.array[i].end_addr);
        if (res_code != 0) {
            close(fd_pagemap);
            close(fd_output);
            clear_page_array(&pg_array);
            return -1;
        }
    }
    clear_page_array(&pg_array);
    close(fd_output);
    close(fd_pagemap);
}