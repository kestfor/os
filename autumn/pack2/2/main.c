#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>// Initialization, should only be called once.
#include <sys/time.h>
#include <unistd.h>

int first_counter = 0;
int second_counter = 0;
int third_counter = 0;
int fourth_counter = 0;

void *first_thread(void *arg) {
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

void *second_thread(void *arg) {
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

void *third_thread(void *arg) {
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

bool is_perm_need() {
    int r = rand();
    return r & 1;
}

void *fourth_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        node *prev = NULL;
        pthread_mutex_lock(&l->head->sync);
        node *curr = l->head;
        node *next = curr->next;
        while (next != NULL) {

            pthread_mutex_lock(&next->sync);

            if (is_perm_need()) {
                if (prev != NULL) {
                    pthread_mutex_lock(&prev->sync);
                    prev->next = next;
                    pthread_mutex_unlock(&prev->sync);
                }
                curr->next = next->next;
                next->next = curr;
                prev = next;
                next = curr->next;
                fourth_counter++;
                pthread_mutex_unlock(&next->sync);
            } else {
                prev = curr;
                curr = next;
                next = curr->next;
                pthread_mutex_unlock(&prev->sync);
            }
        }
        pthread_mutex_unlock(&curr->sync);

    }
}




int main() {
    srand(time(NULL));
    char buff[100];
    list *l = list_init();
    for (int i = 0; i < 1000000; i++) {
        add(l, buff);
    }
    pthread_t first, second, third;
    pthread_t perm[3];
    pthread_create(&first, NULL, first_thread, l);
    pthread_create(&second, NULL, second_thread, l);
    pthread_create(&third, NULL, third_thread, l);
//
//    for (int i = 0; i < 3; i++) {
//        pthread_create(&perm[i], NULL, fourth_thread, l);
//    }

    while (true) {
        sleep(1);
        printf("%d, %d, %d, %d\n", first_counter, second_counter, third_counter, fourth_counter);

    }

    clear(l);
}