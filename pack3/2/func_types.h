#ifndef FUNC_TYPES_H
#define FUNC_TYPES_H


enum func_types {
    make_dir,
    print_dir,
    remove_dir,
    make_file,
    print_file,
    remove_file,
    make_symbolic_link,
    print_symbolic_link,
    print_file_from_symbolic_link,
    remove_symbolic_link,
    make_hard_link,
    remove_hard_link,
    print_mode,
    change_mode
};

typedef struct func_info {
    char *name;
    int func_type;
} func_info;

func_info available_functions[] = {
        {"make_dir",                      make_dir},
        {"print_dir",                     print_dir},
        {"remove_dir",                    remove_dir},
        {"make_file",                     make_file},
        {"print_file",                    print_file},
        {"remove_file",                   remove_file},
        {"make_symbolic_link",            make_symbolic_link},
        {"print_symbolic_link",           print_symbolic_link},
        {"print_file_from_symbolic_link", print_file_from_symbolic_link},
        {"remove_symbolic_link",          remove_symbolic_link},
        {"make_hard_link",                make_hard_link},
        {"remove_hard_link",              remove_hard_link},
        {"print_mode",                    print_mode},
        {"change_mode",                   change_mode}};

#endif //FUNC_TYPES_H
