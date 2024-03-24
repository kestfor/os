#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define BUFF_SIZE 4096
char buffer[BUFF_SIZE];

void reverse(char *buf, size_t len) {
    char tmp;
    for (size_t i = 0; i < len / 2; i++) {
        tmp = buf[i];
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = tmp;
    }
}

void add_name(char *path, char *name) {
    size_t path_len = strlen(path);
    size_t name_len = strlen(name);
    path[path_len] = '/';
    path[path_len + 1] = '\0';
    strncat(path, name, name_len);
}

int reverse_copy(const char *from, const char *to) {

    printf("copy %s to %s\n", from, to);

    int fd_from;
    int fd_to;
    fd_from = open(from, O_RDONLY, 777);

    if (fd_from < 0) {
        perror("cant open source file");
        return -1;
    }

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL);
    if (fd_to < 0) {
        perror("cant open destination file");
        return -1;
    }
    struct stat stat_from;
    fstat(fd_from, &stat_from);
    long start = stat_from.st_size;
    while (true) {
        start = start < BUFF_SIZE ? 0 : start - BUFF_SIZE;

        lseek(fd_from, start, 0);

        read(fd_from, buffer, BUFF_SIZE);

        reverse(buffer, BUFF_SIZE);
        write(fd_to, buffer, BUFF_SIZE);

        if (start == 0) {
            break;
        }
    }

    close(fd_from);
    close(fd_to);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }

    char *og_dir_name = malloc(sizeof(char) * 512);
    char *new_dir_name = malloc(sizeof(char) * 512);

    if (new_dir_name != getcwd(new_dir_name, BUFF_SIZE)) {
        free(new_dir_name);
        perror("cant read current dir");
        return -1;
    }

    DIR *pDirstream = opendir(argv[1]);
    if (pDirstream == NULL) {
        return -1;
    }

    strncpy(og_dir_name, argv[1], strlen(argv[1]));
    char *del_pt = strrchr(argv[1], '/');
    char tmp[512];
    size_t tmp_len;
    if (del_pt != NULL) {
        tmp_len = strlen(del_pt) - 1;
        strncpy(tmp, del_pt + 1, tmp_len);
    } else {
        tmp_len = strlen(argv[1]);
        strncpy(tmp, argv[1], tmp_len);
    }


    reverse(tmp, tmp_len);
    add_name(new_dir_name, tmp);


    int res = mkdir(new_dir_name, S_IWRITE | S_IREAD | S_IEXEC);
//    if (res != 0 && res != EEXIST) {
//        perror("cant create directory");
//        return -1;
//    }

    size_t end_ind = strlen(og_dir_name);
    char *og_file_path = og_dir_name;
    char *new_file_path = new_dir_name;
    for (struct dirent *entry = readdir(pDirstream); entry != NULL; entry = readdir(pDirstream)) {
        if (entry->d_type == DT_REG) {
            og_file_path[end_ind] = '\0';

            add_name(og_file_path, entry->d_name);

            reverse(entry->d_name, strlen(entry->d_name));

            add_name(new_file_path, entry->d_name);

            res = reverse_copy(og_file_path, new_file_path);
            if (res != 0) {
                return -1;
            }
        }
    }

    closedir(pDirstream);
    free(new_dir_name);
    free(og_dir_name);
}