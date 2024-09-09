#ifndef NAMES_H
#define NAMES_H

int make_dir_func(char *dirname, unsigned int mode);

int print_dir_func(char *dirname);

int remove_dir_func(char *dirname);

int make_file_func(char *filename);

int print_file_func(char *filename);

int remove_file_func(char *filename);

int make_symbolic_link_func(char *filename, char *link_name);

int print_symbolic_link_func(char *link_name);

int print_file_from_symbolic_link_func(char *link_name);

int remove_symbolic_link_func(char *remove_symbolic_link);

int make_hard_link_func(char *filename, char *link_name);

int remove_hard_link_func(char *link_name);

int print_mode_func(char *filename);

int change_mode_func(char *filename, unsigned int permission_bits);

#endif //NAMES_H
