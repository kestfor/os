#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>
#include <stdbool.h>

int first_counter = 0;
int second_counter = 0;
int third_counter = 0;

void first_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        int c = 0;
        node *curr = l->head;
        node *next = curr->next;
        pthread_mutex_lock(&curr->sync);
        while (next != NULL) {

            pthread_mutex_lock(&next->sync);

            if (strlen(curr->value) < strlen(next->value)) {
                c++;
            }

            pthread_mutex_unlock(&curr->sync);
            curr = next;
            next = curr->next;
        }
        pthread_mutex_unlock(&curr->sync);
        first_counter++;
    }
}

void second_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        int c = 0;
        node *curr = l->head;
        node *next = curr->next;
        pthread_mutex_lock(&curr->sync);
        while (next != NULL) {

            pthread_mutex_lock(&next->sync);

            if (strlen(curr->value) > strlen(next->value)) {
                c++;
            }

            pthread_mutex_unlock(&curr->sync);
            curr = next;
            next = curr->next;
        }
        pthread_mutex_unlock(&curr->sync);
        second_counter++;
    }
}

void third_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        int c = 0;
        node *curr = l->head;
        node *next = curr->next;
        pthread_mutex_lock(&curr->sync);
        while (next != NULL) {

            pthread_mutex_lock(&next->sync);

            if (strlen(curr->value) == strlen(next->value)) {
                c++;
            }

            pthread_mutex_unlock(&curr->sync);
            curr = next;
            next = curr->next;
        }
        pthread_mutex_unlock(&curr->sync);
        third_counter++;
    }
}




int main() {
    char buff[100];
    list *l = list_init();
    for (int i = 0; i < 1000000; i++) {
        add(l, buff);
    }
    clear(l);
}