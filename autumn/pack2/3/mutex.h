#ifndef PACK2_MUTEX_H
#define PACK2_MUTEX_H
#include <stdatomic.h>
#include <linux/futex.h>
#include <stdbool.h>
#include <linux/futex.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct mutex_t {
    uint32_t lock;
} mutex_t;


typedef struct mutex_t mutex;

void mutex_init(mutex *m);
void lock(mutex *m);
void unlock(mutex *m);

#endif //PACK2_MUTEX_H
