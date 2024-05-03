#include <stdio.h>
#include <stdlib.h>
#include "func_decl.h"
#include "func_types.h"
#include "string.h"
#include <limits.h>

enum func_types get_func_type(char *func_name) {
    char *pt = strrchr(func_name, '/');
    if (pt != NULL) {
        func_name = pt + 1;
    }
    size_t funcs_num = sizeof(available_functions) / sizeof(available_functions[0]);
    for (int i = 0; i < funcs_num; i++) {
        if (strcmp(func_name, available_functions[i].name) == 0) {
            return available_functions[i].func_type;
        }
    }
    return -1;
}

void args_num_mismatch() {
    printf("Argument number mismatch\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 0;
    }

    enum func_types func_type = get_func_type(argv[0]);
    switch (func_type) {
        case make_dir: {
            if (argc != 3) {
                args_num_mismatch();
                return -1;
            }
            char *tmp;
            long mode = strtol(argv[2], &tmp, 10);
            if (mode == LLONG_MAX || mode == LLONG_MIN) {
                perror("can't parse permissions");
                return -1;
            }
            return make_dir_func(argv[1], mode);
        }
        case print_dir:
            return print_dir_func(argv[1]);
        case remove_dir:
            return remove_dir_func(argv[1]);
        case make_file:
            return make_file_func(argv[1]);
        case print_file:
            return print_file_func(argv[1]);
        case remove_file:
            return remove_file_func(argv[1]);
        case make_symbolic_link:
            if (argc != 3) {
                args_num_mismatch();
                return -1;
            }
            return make_symbolic_link_func(argv[1], argv[2]);
        case print_symbolic_link:
            return print_symbolic_link_func(argv[1]);
        case print_file_from_symbolic_link:
            return print_file_from_symbolic_link_func(argv[1]);
        case remove_symbolic_link:
            return remove_symbolic_link_func(argv[1]);
        case make_hard_link:
            if (argc != 3) {
                args_num_mismatch();
                return -1;
            }
            return make_hard_link_func(argv[1], argv[2]);
        case remove_hard_link:
            return remove_hard_link_func(argv[1]);
        case print_mode:
            return print_mode_func(argv[1]);
        case change_mode: {
            char *tmp;
            long mode = strtol(argv[2], &tmp, 16);
            if (mode == LLONG_MAX || mode == LLONG_MIN) {
                perror("can't parse permissions");
                return -1;
            }
            return change_mode_func(argv[1], mode);
        }
        default:
            printf("invalid function\n");
            return -1;
    }
}