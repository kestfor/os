#ifndef PACK2_LIST_H
#define PACK2_LIST_H
#include <pthread.h>


typedef struct _node {
    char value[100];
    struct _node* next;
    pthread_rwlock_t sync;
} node;

typedef struct _list {
    node *head;
} list;

list *list_init();
void add(list *l, char *val);
void clear(list *l);
void rdlock(node *n);
void wrlock(node *n);
void unlock(node *n);

#endif //PACK2_LIST_H
