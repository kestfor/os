#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

typedef void *(*start_routine_t) (void*);

typedef struct thread thread_struct;

typedef thread_struct* thread_t;

int thread_create(thread_t *th, start_routine_t start_routine, void *args);
int thread_join(thread_t th, void **result);


#endif //THREAD_H
