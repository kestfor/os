#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int global_initialized = 0;
int global_uninitialized;
const int global_const;

void func() {
    static int static_var;
    int local_var;
    const int const_var;

    printf("static var: %p\n"
           "local var: %p\n"
           "const var: %p\n"
           "global initialized: %p\n"
           "global uninitialized: %p\n"
           "global const: %p\n", &static_var, &local_var, &const_var, &global_initialized, &global_uninitialized, &global_const);
}


int *get_local_var_addr() {
    int var;
    return &var;
}

void buff_func() {
    char tmp[12] = "hello world";
    char *buff = malloc(sizeof(char) * 100);
    memcpy(buff, tmp, 11);
    printf("buff 1 before free: %s\n", buff);
    free(buff);
    printf("buff 1 after free: %s\n", buff);
    char *buff2 = malloc(sizeof(char) * 100);
    memcpy(buff2, tmp, 11);
    printf("buff 2 before free: %s\n", buff2);
    buff2 += 50;
    free(buff2);
    printf("buff 2 after free: %s\n", buff2);
}

int main() {
    func();

    printf("\nlocal var from func: %p\n\n", get_local_var_addr());
//    buff_func();

    char *env = getenv("test");
    setenv("test", "2", true);
    printf("env var before: %s\n", env);
    env = getenv("test");
    printf("env var after: %s\n", env);
    sleep(100);
}