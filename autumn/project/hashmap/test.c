#include "hashmap.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

HashMap *map;

void *insert1() {
    for (int i =0; i<1000; i++) {
        insert(map, "key1", "data1");
    }
    return NULL;
}

void *insert2() {
    for (int i =0; i<1000; i++) {
        insert(map, "key2", "data2");
    }
    return NULL;
}

void *insert3() {
    for (int i =0; i<1000; i++) {
        insert(map, "key3", "data3");
    }
    return NULL;
}



int main() {
    map = create_hashmap();
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, insert1, NULL);
    pthread_create(&t2, NULL, insert2, NULL);
    pthread_create(&t3, NULL, insert3, NULL);

    cached_data val1;
    bool ok =  get(map, "key1", &val1);
    if (ok) {
        printf("value: %s\n", val1.data);
    }
    free_hashmap(map);
}