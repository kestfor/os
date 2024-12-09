#include "channel.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 256

typedef struct channel {
    char *data;
    size_t capacity;
    size_t actual_len;
    bool whole;
    pthread_cond_t cond_var;
    pthread_mutex_t cond_mutex;
} channel;

size_t channel_get_size(channel *ch) {
    pthread_mutex_lock(&ch->cond_mutex);
    const size_t len = ch->actual_len;
    pthread_mutex_unlock(&ch->cond_mutex);
    return len;
}

size_t max(const size_t first, const size_t second) {
    return first > second ? first : second;
}

channel *channel_create() {
    channel *ch = malloc(sizeof(channel));
    ch->data = malloc(INITIAL_CAPACITY);
    ch->actual_len = 0;
    ch->capacity = INITIAL_CAPACITY;
    ch->whole = false;
    pthread_cond_init(&ch->cond_var, NULL);
    pthread_mutex_init(&ch->cond_mutex, NULL);
    return ch;
}

void channel_clear(channel *ch) {
    free(ch->data);
    pthread_cond_destroy(&ch->cond_var);
    pthread_mutex_destroy(&ch->cond_mutex);
    free(ch);
}

void channel_wait_for_data(channel *ch, const int offset) {
    pthread_mutex_lock(&ch->cond_mutex);
    if (ch->actual_len > offset || ch->whole) {
        pthread_mutex_unlock(&ch->cond_mutex);
        return;
    }
    pthread_cond_wait(&ch->cond_var, &ch->cond_mutex);
    pthread_mutex_unlock(&ch->cond_mutex);
}


void channel_set_whole(channel *ch) {
    pthread_mutex_lock(&ch->cond_mutex);
    ch->whole = true;
    pthread_cond_broadcast(&ch->cond_var);
    pthread_mutex_unlock(&ch->cond_mutex);
}

int _add_inner(channel *ch, const char *data, const size_t size) {
    ch->whole = false;

    if (size + ch->actual_len > ch->capacity) {
        void *tmp = ch->data;
        const size_t new_len = max(ch->capacity * 2, size + ch->actual_len + 1);
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
    return 0;
}

int channel_add(channel *ch, const char *data, const size_t size) {
    pthread_mutex_lock(&ch->cond_mutex);
    const int res = _add_inner(ch, data, size);
    pthread_cond_broadcast(&ch->cond_var);
    pthread_mutex_unlock(&ch->cond_mutex);
    return res;
}


int channel_write(channel *ch, const char *data, const size_t size) {
    pthread_mutex_lock(&ch->cond_mutex);
    ch->actual_len = 0;
    const int res = _add_inner(ch, data, size);
    pthread_cond_broadcast(&ch->cond_var);
    pthread_mutex_unlock(&ch->cond_mutex);
    return res;
}

bool channel_is_whole(channel *ch) {
    pthread_mutex_lock(&ch->cond_mutex);
    const bool res = ch->whole;
    pthread_mutex_unlock(&ch->cond_mutex);
    return res;
}

void channel_set_empty(channel *ch) {
    pthread_mutex_lock(&ch->cond_mutex);
    ch->actual_len = 0;
    pthread_mutex_unlock(&ch->cond_mutex);
}

bool channel_read_available(channel *ch, char *dest, const int offset, const int size, size_t *actual_read_num) {
    pthread_mutex_lock(&ch->cond_mutex);
    bool end = ch->whole;
    if (offset >= ch->actual_len) {
        if (actual_read_num != NULL) {
            *actual_read_num = 0;
        }
        pthread_mutex_unlock(&ch->cond_mutex);
        return end;
    }
    size_t read_num = 0;
    if (size < ch->actual_len - offset) {
        end = false;
        read_num = size;
    } else {
        read_num = ch->actual_len - offset;
    }
    memcpy(dest, ch->data + offset, read_num);
    pthread_mutex_unlock(&ch->cond_mutex);
    if (actual_read_num != NULL) {
        *actual_read_num = read_num;
    }
    return end;
}