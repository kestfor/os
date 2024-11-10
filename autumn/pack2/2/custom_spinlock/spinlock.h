

#ifndef SPINLOCK_H
#define SPINLOCK_H
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

typedef struct spinlock_t {
    int lock;
} spinlock_t;


typedef struct spinlock_t spinlock;

void spinlock_init(spinlock *m);
void lock_spinlock(spinlock *m);
void unlock_spinlock(spinlock *m);


#endif //SPINLOCK_H
