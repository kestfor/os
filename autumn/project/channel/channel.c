#include "channel.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 256

typedef struct channel {
    pthread_rwlock_t rwlock;
    char *data;
    size_t capacity;
    size_t actual_len;
    bool whole;
} channel;

size_t get_size(channel *ch) {
    return ch->actual_len;
}

size_t max(size_t first, size_t second) {
    return first > second ? first : second;
}

size_t min(size_t first, size_t second) {
    return first < second ? first : second;
}

channel *new_channel() {
    channel *ch = malloc(sizeof(channel));
    pthread_rwlock_init(&ch->rwlock, NULL);
    ch->data = malloc(INITIAL_CAPACITY);
    ch->actual_len = 0;
    ch->capacity = INITIAL_CAPACITY;
    ch->whole = false;
    return ch;
}

void clear_channel(channel *ch) {
    free(ch->data);
    free(ch);
}

void set_whole(channel *ch) {
    pthread_rwlock_wrlock(&ch->rwlock);
    ch->whole = true;
    pthread_rwlock_unlock(&ch->rwlock);
}

int add_to_channel(channel *ch, const char *data, size_t size) {
    pthread_rwlock_wrlock(&ch->rwlock);
    ch->whole = false;

    if (size + ch->actual_len > ch->capacity) {
        void *tmp = ch->data;
        size_t new_len = max(ch->capacity * 2, size + ch->actual_len + 1);
        ch->data = realloc(ch->data, new_len);
        if (ch->data == NULL) {
            free(tmp);
            perror("realloc");
            return -1;
        }
        ch->capacity = new_len;
    }

    memcpy(ch->data + ch->actual_len, data, size);
    ch->actual_len += size;

    pthread_rwlock_unlock(&ch->rwlock);
    return 0;
}

int write_to_channel(channel *ch, const char *data, size_t size) {
    pthread_rwlock_wrlock(&ch->rwlock);
    ch->actual_len = 0;
    pthread_rwlock_unlock(&ch->rwlock);
    return add_to_channel(ch, data, size);
}

bool is_whole(channel *ch) {
    pthread_rwlock_rdlock(&ch->rwlock);
    bool res = ch->whole;
    pthread_rwlock_unlock(&ch->rwlock);
    return res;
}

void empty_channel(channel *ch) {
    pthread_rwlock_rdlock(&ch->rwlock);
    ch->actual_len = 0;
    pthread_rwlock_unlock(&ch->rwlock);
}

bool read_available(channel *ch, char *dest, int offset, int size, int *actual_read_num) {
    pthread_rwlock_rdlock(&ch->rwlock);
    if (offset > ch->actual_len) {
        return 0;
    }
    bool end = ch->whole;
    size_t read_num = 0;
    if (size < ch->actual_len - offset) {
        end = false;
        read_num = size;
    } else {
        read_num = ch->actual_len - offset;
    }
    memcpy(dest, ch->data + offset, read_num);
    pthread_rwlock_unlock(&ch->rwlock);
    if (actual_read_num != NULL) {
        *actual_read_num = read_num;
    }
    return end;
}