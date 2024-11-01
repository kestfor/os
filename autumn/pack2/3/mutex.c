#include "mutex.h"

int futex(uint32_t *addr, int futex_op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3) {
    return (int) syscall(SYS_futex, addr, futex_op, val, timeout, uaddr2, val3);
}

bool _try_lock(mutex *m) {
    int one = 1;
    return atomic_compare_exchange_strong(&(m->lock), &one, 0);
}

bool _try_unlock(mutex *m) {
    int zero = 0;
    return atomic_compare_exchange_strong(&(m->lock), &zero, 1);
}

void _park_thread(mutex *m) {
    int err = futex(&(m->lock), FUTEX_WAIT, 0, NULL, NULL, 0);
    if (err == -1 && errno != EAGAIN) {
        printf("%d", errno);
        printf("futex wait failed");
        abort();
    }
}

void _wake_threads(mutex *m) {
    int err = futex(&(m->lock), FUTEX_WAKE, 1, NULL, NULL, 0);
    if (err == -1) {
        printf("futex wake failed");
        abort();
    }
}

int _fast_lock(mutex *m) {
    while (true) {
        if (_try_lock(m)) {
            break;
        } else {
            _park_thread(m);
        }
    }
    return 0;
}

int _fast_unlock(mutex *m) {
    if (_try_unlock(m)) {
        _wake_threads(m);
    }
    return 0;
}

int _error_checking_lock(mutex *m) {
    int tid = gettid();
    while (true) {

        if (_try_lock(m)) {
            m->owner = tid;
            break;
        }

        if (tid == m->owner) {
            return ALREADY_LOCKED_ERROR;
        }

        _park_thread(m);

    }
    return 0;
}

int _error_checking_unlock(mutex *m) {
    int tid = gettid();
    if (tid != m->owner) {
        return PERMISSION_ERROR;
    } else {
        m->owner = -1;
        return _fast_unlock(m);
    }
}

int _recursive_lock(mutex *m) {
    int tid = gettid();
    while (true) {
        if (_try_lock(m)) {
            m->counter = 1;
            m->owner = tid;
            break;
        }

        if (m->owner == tid) {
            m->counter++;
            break;
        } else {
            _park_thread(m);
        }
    }
    return m->counter;
}

int _recursive_unlock(mutex *m) {
    int tid = gettid();

    if (tid != m->owner) {
        return PERMISSION_ERROR;
    } else {
        m->counter--;
    }

    if (m->counter <= 0) {
        m->counter = 0;
        m->owner = -1;
        return _fast_unlock(m);
    } else {
        return m->counter;
    }
}

void mutex_init(mutex *m, enum mutex_kind kind) {
    m->lock = 1;
    m->kind = kind;
    m->owner = -1;
    m->counter = 0;
}

int lock_mutex(mutex *m) {
    switch (m->kind) {
        case FAST:
            return _fast_lock(m);
        case RECURSIVE:
            return _recursive_lock(m);
        case ERROR_CHECKING:
            return _error_checking_lock(m);
    }
}

int unlock_mutex(mutex *m) {
    switch (m->kind) {
        case FAST:
            return _fast_unlock(m);
        case RECURSIVE:
            return _recursive_unlock(m);
        case ERROR_CHECKING:
            return _error_checking_unlock(m);
    }
}