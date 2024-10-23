#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint-gcc.h>
#include <time.h>
#include <stdbool.h>
#include "hashmap.h"
#include <pthread.h>

#define TABLE_SIZE 1000

typedef struct HashNode {
    char* key;
    cached_data data;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    pthread_rwlock_t rwlock;
    HashNode** table;
} HashMap;

uint32_t fnv1a_hash(const char *str) {
    uint32_t hash = 2166136261u;
    const uint32_t fnv_prime = 16777619u;

    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= fnv_prime;
        str++;
    }

    return hash;
}

uint32_t hash(const char* key) {
    return fnv1a_hash(key) % TABLE_SIZE;
}

HashMap* create_hashmap() {
    HashMap* hashmap = malloc(sizeof(HashMap));
    hashmap->table = malloc(TABLE_SIZE * sizeof(HashNode*));
    pthread_rwlock_init(&hashmap->rwlock, NULL);

    for (int i = 0; i < TABLE_SIZE; i++) {
        hashmap->table[i] = NULL;
    }

    return hashmap;
}

void clear_node(HashNode *node) {
    free(node->key);
    clear_channel(node->data.data);
    free(node);
}

void insert_item(HashMap* hashmap, const char* key, const char *value) {
    unsigned int index = hash(key);

    channel *new_ch = new_channel();
    cached_data data = {time(NULL), new_ch};
    if (value != NULL) {
        write_to_channel(new_ch, value, strlen(value));
    }

    HashNode* newNode = malloc(sizeof(HashNode));
    newNode->key = strdup(key);
    newNode->data = data;
    newNode->next = NULL;

    pthread_rwlock_wrlock(&hashmap->rwlock);
    if (hashmap->table[index] == NULL) {
        hashmap->table[index] = newNode;
    } else {
        HashNode* temp = hashmap->table[index];
        while (temp) {
            if (strcmp(temp->key, key) == 0) {
                temp->data = data;
                pthread_rwlock_unlock(&hashmap->rwlock);
                free(newNode->key);
                free(newNode);
                return;
            }
            temp = temp->next;
        }
        newNode->next = hashmap->table[index];
        hashmap->table[index] = newNode;
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
}

bool get_item(HashMap* hashmap, const char* key, cached_data *data) {
    unsigned int index = hash(key);

    pthread_rwlock_rdlock(&hashmap->rwlock);
    HashNode* node = hashmap->table[index];

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

bool capture_item(HashMap* hashmap, const char* key, cached_data *data) {
    unsigned int index = hash(key);

    pthread_rwlock_rdlock(&hashmap->rwlock);
    HashNode* node = hashmap->table[index];

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

void release_item(HashMap* hashmap, const char* key) {
    pthread_rwlock_unlock(&hashmap->rwlock);
}

void delete_item(HashMap* hashmap, const char* key) {
    unsigned int index = hash(key);

    pthread_rwlock_wrlock(&hashmap->rwlock);
    HashNode* node = hashmap->table[index];
    HashNode* prev = NULL;

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                hashmap->table[index] = node->next;
            }
            pthread_rwlock_unlock(&hashmap->rwlock);
            clear_node(node);
            return;
        }
        prev = node;
        node = node->next;
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
}

void clear_old(HashMap *hashmap, time_t last_time) {
    pthread_rwlock_wrlock(&hashmap->rwlock);
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* node = hashmap->table[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            if (temp->data.cached_time < last_time) {
                clear_node(temp);
                if (hashmap->table[i] == temp) {
                    hashmap->table[i] = node;
                }
            }
        }
    }
    pthread_rwlock_unlock(&hashmap->rwlock);
}

void free_hashmap(HashMap* hashmap) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* node = hashmap->table[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            clear_node(temp);
        }
    }
    free(hashmap->table);
    pthread_rwlock_destroy(&hashmap->rwlock);
    free(hashmap);
}