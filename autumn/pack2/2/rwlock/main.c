#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

int first_counter = 0;
int second_counter = 0;
int third_counter = 0;
int fourth_counter = 0;

pthread_mutex_t counter_mutex;

void *first_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        int c = 0;
        node *curr = l->head;
        rdlock(curr);
        node *next = curr->next;
        while (next != NULL) {

            rdlock(next);

            if (strlen(curr->value) < strlen(next->value)) {
                c++;
            }

            unlock(curr);
            curr = next;
            next = curr->next;
        }
        unlock(curr);
        first_counter++;
    }
}

void *second_thread(void *arg) {
    list *l = (list *) arg;
    while (true) {
        int c = 0;
        node *curr = l->head;
        rdlock(curr);
        node *next = curr->next;
        while (next != NULL) {

            rdlock(next);

            if (strlen(curr->value) > strlen(next->value)) {
                c++;
            }

            unlock(curr);
            curr = next;
            next = curr->next;
        }
        unlock(curr);
        second_counter++;
    }
}

void *third_thread(void *arg) {
    list *l = (list *) arg;
    int thread_id = gettid();
    while (true) {
        int c = 0;
        node *curr = l->head;
        //printf("try lock %p, %d\n", curr, thread_id);
        rdlock(curr);

        //printf("locked %p, %d\n", curr, thread_id);
        node *next = curr->next;
        while (next != NULL) {

//            printf("try lock %p, %d\n", next, thread_id);
            rdlock(next);
            //printf("locked %p, %d\n", next, thread_id);

            if (strlen(curr->value) == strlen(next->value)) {
                c++;
            }

            unlock(curr);
            //printf("unlocked %p, %d\n", curr, thread_id);
            curr = next;
            next = curr->next;
        }
        unlock(curr);
        //printf("unlocked %p, %d\n", curr, thread_id);
        third_counter++;
    }
}

bool is_perm_need() {
    int r = rand();
    return r & 1;
}

void print_list(list *l) {
    node *curr = l->head;
    while (curr != NULL) {
        printf("%s ", curr->value);
        curr = curr->next;
    }
    printf("\n");
}

void *fourth_thread(void *arg) {
    list *l = (list *) arg;
    //int thread_id = gettid();
    while (true) {
        node *prev = NULL;
        node *curr = l->head;
        //printf("[perm] try lock %p, %d\n", curr, thread_id);
        wrlock(curr);
//        printf("[perm] locked %p, %d\n", l->head, thread_id);
        node *next = curr->next;
        while (next != NULL) {

//            printf("[perm] try lock %p, %d\n", next, thread_id);
            wrlock(next);
//            printf("[perm] locked %p, %d\n", next, thread_id);

            if (is_perm_need()) {
                if (prev != NULL) {
                    prev->next = next;
//                    pthread_mutex_unlock(&prev->sync);
                }

                if (l->head == curr) {
                    l->head = next;
                }
//                printf("[perm] permutation %d\n", thread_id);
                curr->next = next->next;
                next->next = curr;

                node *tmp = curr;
                curr = next;
                next = tmp;
                pthread_mutex_lock(&counter_mutex);

                fourth_counter++;

                pthread_mutex_unlock(&counter_mutex);

            }
            if (prev != NULL) {
                unlock(prev);
//                printf("[perm] unlocked %p, %d\n", prev, thread_id);
            }

            prev = curr;
            curr = next;
            next = next->next;
//            printf("%p, %p, %p, %d\n", prev, curr, next, thread_id);
  //          printf("[perm] next tick %d\n", thread_id);
        }
//        printf("exited %p, %p, %p, %d\n", prev, curr, next, thread_id);
        if (prev != NULL) {
            unlock(prev);
  //          printf("[perm] unlocked %p, %d\n", prev, thread_id);
        }
        unlock(curr);
//        printf("[perm] unlocked %p, %d\n", curr, thread_id);
    }
}

int main() {
    srand(time(NULL));
    char buff[100];
    list *l = list_init();
    for (int i = 0; i < 10000; i++) {
        add(l, buff);
    }
    pthread_t first, second, third;
    pthread_t perm[3];
    pthread_create(&first, NULL, first_thread, l);
    pthread_create(&second, NULL, second_thread, l);
    pthread_create(&third, NULL, third_thread, l);


    pthread_mutex_init(&counter_mutex, NULL);
    for (int i = 0; i < 2; i++) {
        pthread_create(&perm[i], NULL, fourth_thread, l);
    }

    sleep(2);
    printf("%d, %d, %d, %d\n", first_counter, second_counter, third_counter, fourth_counter);

    clear(l);
}