#include "func_decl.h"
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int make_dir_func(char *dirname, unsigned int mode) {
    return mkdir(dirname, mode);
}

int print_dir_func(char *dirname) {
    DIR *dir_stream = opendir(dirname);
    if (dir_stream != NULL) {
        for (struct dirent *entry = readdir(dir_stream); entry != NULL; entry = readdir(dir_stream)) {
            printf("%s\n", entry->d_name);
        }
        closedir(dir_stream);
    } else {
        perror("Couldn't open the directory");
        return -1;
    }
    return 0;
}

int remove_dir_func(char *dirname) {
    if (rmdir(dirname) != 0) {
        perror("Couldn't remove the directory");
        return -1;
    }
    return 0;
}

int make_file_func(char *filename) {
    int fd = open(filename, O_CREAT);
    if (fd < 0) {
        perror("Couldn't create file");
        return -1;
    }
    return 0;
}

int print_file_func(char *filename) {
    char buff[2048];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Couldn't create file");
        return -1;
    } else {
        size_t n;
        while ((n = read(fd, buff, 2048))) {
            write(STDOUT_FILENO, buff, n);
        }
        return 0;
    }
}

int remove_file_func(char *filename) {
    if (unlink(filename) != 0) {
        perror("Couldn't remove file");
        return -1;
    } else {
        return 0;
    }
}

int make_symbolic_link_func(char *filename, char *link_name) {
    if (symlink(filename, link_name) != 0) {
        perror("Couldn't make symbolic link");
        return -1;
    }
    return 0;
}

char *get_file_from_link(char *link_name) {
    struct stat stat_buff;
    char *buf;
    ssize_t read, buff_size;
    if (stat(link_name, &stat_buff) == -1) {
        perror("Couldn't read symbolic link stat");
        return NULL;
    }

    if (stat_buff.st_size == 0) {
        buff_size = PATH_MAX;
    } else {
        buff_size = stat_buff.st_size + 1;
    }

    buf = malloc(sizeof(char) * buff_size);
    read = readlink(link_name, buf, buff_size);

    if (read != buff_size) {
        perror("Couldn't read full symbolic link");
        return NULL;
    }
    return buf;
}

int print_symbolic_link_func(char *link_name) {
    char *file_name = get_file_from_link(link_name);
    if (file_name != NULL) {
        printf("%s", file_name);
        free(file_name);
        return 0;
    } else {
        return -1;
    }
}

int print_file_from_symbolic_link_func(char *link_name) {
    char *file_name = get_file_from_link(link_name);
    if (file_name != NULL) {
        print_file_func(file_name);
        free(file_name);
        return 0;
    } else {
        return -1;
    }
}

int remove_symbolic_link_func(char *link_name) {
    if (unlink(link_name) != 0) {
        perror("Couldn't remove symbolic link");
        return -1;
    } else {
        return 0;
    }
}

int make_hard_link_func(char *filename, char *link_name) {
    if (link(filename, link_name) != 0) {
        perror("Couldn't remove symbolic link");
        return -1;
    }
    return 0;
}

int remove_hard_link_func(char *link_name) {
    if (unlink(link_name) != 0) {
        perror("Couldn't remove hard link");
        return -1;
    } else {
        return 0;
    }
}

int get_mode_string(unsigned int permission_bits, char *buffer) {
    memset(buffer, 0, sizeof(char) * 10);
    int mods_num = 9;
    int mods[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    for (int i = 0; i < mods_num; i++) {
        if (permission_bits & mods[i]) {
            if (i % 3 == 0) {
                buffer[i] = 'r';
            } else if (i % 3 == 1) {
                buffer[i] = 'w';
            } else if (i % 3 == 2) {
                buffer[i] = 'x';
            }
        } else {
            buffer[i] = '-';
        }
    }
    return 0;
}

int print_mode_func(char *filename) {
    struct stat stat_buff;
    if (stat(filename, &stat_buff) == -1) {
        perror("Couldn't read file stat");
        return -1;
    }
    char buff[10];
    get_mode_string(stat_buff.st_mode, buff);
    printf("Hard links num: %ld, permissions: %s\n", stat_buff.st_nlink, buff);
    return 0;
}

int change_mode_func(char *filename, unsigned int permission_bits) {
    if (chmod(filename, permission_bits) != 0) {
        perror("Couldn't change file permissions");
        return -1;
    }
    return 0;
}

