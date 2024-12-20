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
    mutex_init(&new->sync);
    new->next = l->head;
    l->head = new;

    strncpy(new->value, val, 100);
}

void clear(list *l) {
    while (l->head != NULL) {
        node *next = l->head->next;
        free(l->head);
        l->head = next;
    }
}

void lock(node *n) {
    lock_mutex(&(n->sync));
}

void unlock(node *n) {
    unlock_mutex(&(n->sync));
}
