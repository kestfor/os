#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int min(int first, int second) {
    return first < second ? first : second;
}

list* list_init() {
    int err;

    list *l = malloc(sizeof(list));
    if (!l) {
        printf("Cannot allocate memory for a queue\n");
        abort();
    }

    l->head = NULL;

    return l;
}

void add(list *l, char *val) {
    node *new = malloc(sizeof(node));
    pthread_rwlock_init(&new->sync, NULL);
    new->next = l->head;
    l->head = new;

    strncpy(new->value, val, 100);
}

void clear(list *l) {
    while (l->head != NULL) {
        node *next = l->head->next;
        pthread_rwlock_destroy(&l->head->sync);
        free(l->head);
        l->head = next;
    }
}

void rdlock(node *n) {
    pthread_rwlock_rdlock(&(n->sync));
}

void wrlock(node *n) {
    pthread_rwlock_wrlock(&(n->sync));
}

void unlock(node *n) {
    pthread_rwlock_unlock(&(n->sync));
}
