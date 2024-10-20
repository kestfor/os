#include <stdio.h>
#include "../2/custom_mutex/mutex.h"
#include <pthread.h>


mutex m;
int counter_var = 0;

void *counter() {
    for (int i = 0; i < 10000; i++) {
        lock_mutex(&m);
        fflush(stdout);
        counter_var++;
        unlock_mutex(&m);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    mutex_init(&m);
    pthread_t first, second;
    pthread_create(&first, NULL, counter, NULL);
    pthread_create(&second, NULL, counter, NULL);

    pthread_join(first, NULL);
    pthread_join(second, NULL);
    printf("%d", counter_var);
}
