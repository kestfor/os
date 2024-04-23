#include <stdio.h>
#include <string.h>
#include "custom_malloc.h"

int main() {
    int *array = my_malloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) {
        array[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        printf("%d ", array[i]);
    }
    char *buff = my_malloc(sizeof(char) * 512);
    memset(buff, 0, 512);
    memcpy(buff, "hello world", 11);
    printf("\n%s", buff);
    int *a = my_malloc(sizeof(int));
    char *b = my_malloc(sizeof(char));
    double *c = my_malloc(sizeof(double));
    my_free(b);
    my_free(a);
    my_free(c);
    my_free(buff);
    my_free(array);
}