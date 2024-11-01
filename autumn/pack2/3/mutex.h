#ifndef PACK2_MUTEX_H
#define PACK2_MUTEX_H
#define _GNU_SOURCE
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

enum mutex_kind {
    FAST,
    RECURSIVE,
    ERROR_CHECKING
};

enum errors {
    PERMISSION_ERROR = -1,
    ALREADY_LOCKED_ERROR = -2,
};


typedef struct mutex_t {
    uint32_t lock;
    enum mutex_kind kind;
    int owner;
    int counter;
} mutex_t;


typedef struct mutex_t mutex;

void mutex_init(mutex *m, enum mutex_kind kind);
int lock_mutex(mutex *m);
int unlock_mutex(mutex *m);

#endif //PACK2_MUTEX_H
