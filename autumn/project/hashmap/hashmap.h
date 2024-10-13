#ifndef PROXY_HASHMAP_H
#define PROXY_HASHMAP_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include "../channel/channel.h"

typedef struct cached_data {
    clock_t cached_time;
    channel *data;
} cached_data;


typedef struct HashMap HashMap;

HashMap* create_hashmap();
void insert_item(HashMap* hashmap, const char* key, const char *value);
bool get_item(HashMap* hashmap, const char* key, cached_data *data); // not thread safe for channel, data received pointer to channel that may be freed by another thread
void delete_item(HashMap* hashmap, const char* key);
void free_hashmap(HashMap* hashmap);
void clear_old(HashMap* hashmap, clock_t last_time);
bool borrow_item(HashMap* hashmap, const char* key, cached_data *data); // thread safe for channel, if item was found, it should be released later
void release_item(HashMap* hashmap, const char* key);
#endif //PROXY_HASHMAP_H
