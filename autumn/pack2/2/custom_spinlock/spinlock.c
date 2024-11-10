#include "spinlock.h"

void spinlock_init(spinlock *s) {
    s->lock = 1;
}

void lock_spinlock(spinlock *s) {
    while (true) {
        int one = 1;
        if (atomic_compare_exchange_strong(&(s->lock), &one, 0)) {
            break;
        }
    }
}

void unlock_spinlock(spinlock *s) {
    int zero = 0;
    atomic_compare_exchange_strong(&(s->lock), &zero, 1);
}