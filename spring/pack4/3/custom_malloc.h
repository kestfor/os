#ifndef CUSTOM_MALLOC_H
#define CUSTOM_MALLOC_H

typedef unsigned long size_t;

void *my_malloc(size_t size);

void my_free(void *addr);

#endif //CUSTOM_MALLOC_H
