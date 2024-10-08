#include "mutex.h"

int futex(uint32_t *addr, int futex_op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3) {
    return (int) syscall(SYS_futex, addr, futex_op, val, timeout, uaddr2, val3);
}

void mutex_init(mutex *m) {
    m->lock = 1;
}

void lock(mutex *m) {
    while (true) {
        int one = 1;
        if (atomic_compare_exchange_strong(&(m->lock), &one, 0)) {
            break;
        }
        int err = futex(&(m->lock), FUTEX_WAIT, 0, NULL, NULL, 0);
        if (err == -1 && errno != EAGAIN) {
            printf("%d", errno);
            printf("futex wait failed");
            abort();
        }
    }
}

void unlock(mutex *m) {
    int zero = 0;
    if (atomic_compare_exchange_strong(&(m->lock), &zero, 1)) {
        int err = futex(&(m->lock), FUTEX_WAKE, 1, NULL, NULL, 0);
        if (err == -1) {
            printf("futex wake failed");
            abort();
        }
    }

}