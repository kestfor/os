#include <stdlib.h>
#include <string.h>
#include <stdint-gcc.h>
#include <time.h>
#include <stdbool.h>
#include "hashmap.h"
#include <pthread.h>
#include <stdio.h>

#define TABLE_SIZE 1000
#define MAX_ITEM_NUM 100000
#define GC_ITER_NUM 4

int min(const int a, const int b) {
    return a < b ? a : b;
}

typedef struct HashNode {
    char *key;
    cached_data data;
    struct HashNode *next;
} HashNode;

typedef struct GarbageCollector {
    int capacity;
    int size;
    int last_checked_index;
    HashNode **garbage;
} GarbageCollector;


typedef struct HashMap {
    int size;
    pthread_rwlock_t rwlock;
    HashNode **table;
    GarbageCollector gc;
} HashMap;

uint32_t fnv1a_hash(const char *str) {
    uint32_t hash = 2166136261u;

    while (*str) {
        const uint32_t fnv_prime = 16777619u;
        hash ^= (unsigned char) *str;
        hash *= fnv_prime;
        str++;
    }

    return hash;
}

void clear_node(HashNode *node) {
    free(node->key);
    channel_clear(node->data.data);
    free(node);
}

// real cleaning
void gc_free_items(GarbageCollector *gc) {
    for (int i = 0; i < gc->size; i++) {
        clear_node(gc->garbage[i]);
        gc->garbage[i] = NULL;
        gc->size--;
    }
}

//inner function, must be locked before (wrlock), appends items to gc to clean after
void hashmap_gc_clear_oldest(HashMap *hashmap) {
    HashNode *oldest = NULL;
    HashNode *parent_of_oldest = NULL;
    int table_ind = -1;
    time_t oldest_time = time(NULL);
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashmap->table[i];
        HashNode *prev = NULL;
        while (node) {
            HashNode *temp = node;
            node = node->next;
            if (temp->data.cached_time <= oldest_time) {
                oldest = temp;
                parent_of_oldest = prev;
                oldest_time = temp->data.cached_time;
                if (hashmap->table[i] == temp) {
                    table_ind = i;
                }
            }
            prev = temp;
        }
    }
    if (oldest != NULL) {
        HashNode *node = oldest->next;
        if (parent_of_oldest != NULL) {
            parent_of_oldest->next = node;
        }
        hashmap->gc.garbage[hashmap->gc.size++] = oldest;
        hashmap->size--;
        if (table_ind != -1) {
            hashmap->table[table_ind] = node;
        }
    }
}

// either appends items to gc or performs cleaning,
// every call of function checks TABLE_SIZE / GC_ITER_NUM num of table_items, and perform cleaning if all table was checked
bool hashmap_gc_do_iter(HashMap *hashmap, const time_t oldest_time) {
    bool cleaned = false;
    if (hashmap->gc.last_checked_index == TABLE_SIZE) {
        gc_free_items(&hashmap->gc);
        hashmap->gc.last_checked_index = 0;
        cleaned = true;
    }
    pthread_rwlock_wrlock(&hashmap->rwlock);

    const int last_index = min(hashmap->gc.last_checked_index + (TABLE_SIZE / GC_ITER_NUM), TABLE_SIZE);
    for (int i = hashmap->gc.last_checked_index; i < last_index; i++) {
        HashNode *parent = NULL;
        HashNode *node = hashmap->table[i];
        while (node) {
            HashNode *temp = node;
            node = node->next;
            if (temp->data.cached_time < oldest_time) {
                hashmap->gc.garbage[hashmap->gc.size++] = temp;
                hashmap->size--;
                if (parent != NULL) {
                    parent->next = node;
                }
                if (hashmap->table[i] == temp) {
                    hashmap->table[i] = node;
                }
            }
            parent = temp;
        }
    }
    hashmap->gc.last_checked_index = last_index;
    pthread_rwlock_unlock(&hashmap->rwlock);
    return cleaned;
}

uint32_t hash(const char *key) {
    return fnv1a_hash(key) % TABLE_SIZE;
}

HashMap *create_hashmap() {
    HashMap *hashmap = malloc(sizeof(HashMap));
    hashmap->table = malloc(TABLE_SIZE * sizeof(HashNode *));
    pthread_rwlock_init(&hashmap->rwlock, NULL);

    hashmap->gc.capacity = MAX_ITEM_NUM;
    hashmap->gc.size = 0;
    hashmap->gc.last_checked_index = 0;
    hashmap->gc.garbage = malloc(sizeof(HashNode *) * hashmap->gc.size);

    for (int i = 0; i < TABLE_SIZE; i++) {
        hashmap->table[i] = NULL;
    }
    hashmap->size = 0;
    return hashmap;
}

//returns true if item was inserted/replaced
bool _resolve_collision(HashMap *hashmap, HashNode *newNode, const unsigned int index, const bool replace) {
    if (hashmap->table[index] == NULL) {
        hashmap->table[index] = newNode;
        hashmap->size++;
        return true;
    }
    HashNode *temp = hashmap->table[index];
    while (temp) {
        if (strcmp(temp->key, newNode->key) == 0) {
            if (replace) {
                temp->data = newNode->data;
                free(newNode->key);
                free(newNode);
                return true;
            }
            return false;
        }
        temp = temp->next;
    }
    if (hashmap->size >= MAX_ITEM_NUM) {
        hashmap_gc_clear_oldest(hashmap);
    }
    newNode->next = hashmap->table[index];
    hashmap->table[index] = newNode;
    hashmap->size++;
    return true;
}

HashNode *_create_new_node(const char *key, const char *value) {
    channel *new_ch = channel_create();
    const cached_data data = {time(NULL), new_ch};
    if (value != NULL) {
        channel_write(new_ch, value, strlen(value));
    }

    HashNode *newNode = malloc(sizeof(HashNode));
    newNode->key = strdup(key);
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

bool insert_item(HashMap *hashmap, const char *key, const char *value) {
    const unsigned int index = hash(key);
    HashNode *newNode = _create_new_node(key, value);
    pthread_rwlock_wrlock(&hashmap->rwlock);
    const bool inserted = _resolve_collision(hashmap, newNode, index, false);
    pthread_rwlock_unlock(&hashmap->rwlock);
    return inserted;
}

bool insert_replace_item(HashMap *hashMap, const char *key, const char *value) {
    const unsigned int index = hash(key);
    HashNode *newNode = _create_new_node(key, value);
    pthread_rwlock_wrlock(&hashMap->rwlock);
    const bool inserted = _resolve_collision(hashMap, newNode, index, true);
    pthread_rwlock_unlock(&hashMap->rwlock);
    return inserted;
}


bool get_item(HashMap *hashmap, const char *key, cached_data *data) {
    const unsigned int index = hash(key);

    pthread_rwlock_rdlock(&hashmap->rwlock);
    const HashNode *node = hashmap->table[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            memcpy(data, &node->data, sizeof(cached_data));
            pthread_rwlock_unlock(&hashmap->rwlock);
            return true;
        }
        node = node->next;
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
    return false;
}

bool capture_item(HashMap *hashmap, const char *key, cached_data *data) {
    const unsigned int index = hash(key);

    pthread_rwlock_rdlock(&hashmap->rwlock);
    const HashNode *node = hashmap->table[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            memcpy(data, &node->data, sizeof(cached_data));
            return true;
        }
        node = node->next;
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
    return false;
}

void release_item(HashMap *hashmap) {
    pthread_rwlock_unlock(&hashmap->rwlock);
}

void delete_item(HashMap *hashmap, const char *key) {
    const unsigned int index = hash(key);

    pthread_rwlock_wrlock(&hashmap->rwlock);
    HashNode *node = hashmap->table[index];
    HashNode *prev = NULL;

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                hashmap->table[index] = node->next;
            }
            hashmap->size--;
            pthread_rwlock_unlock(&hashmap->rwlock);
            clear_node(node);
            return;
        }
        prev = node;
        node = node->next;
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
}

void clear_old(HashMap *hashmap, const time_t last_time) {
    pthread_rwlock_wrlock(&hashmap->rwlock);
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashmap->table[i];
        while (node) {
            HashNode *temp = node;
            node = node->next;
            if (temp->data.cached_time < last_time) {
                clear_node(temp);
                hashmap->size--;
                if (hashmap->table[i] == temp) {
                    hashmap->table[i] = node;
                }
            }
        }
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
}


void free_hashmap(HashMap *hashmap) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashmap->table[i];
        while (node) {
            HashNode *temp = node;
            node = node->next;
            clear_node(temp);
        }
    }
    free(hashmap->gc.garbage);
    free(hashmap->table);
    pthread_rwlock_destroy(&hashmap->rwlock);
    free(hashmap);
}
