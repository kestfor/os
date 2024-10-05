#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <pthread.h>
#include <sched.h>

#include "mutex/queue.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

void set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	if (err) {
		printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return;
	}

	printf("set_cpu: set cpu %d\n", n);
}

void *reader(void *arg) {
	int expected = 0;
	queue_t *q = (queue_t *)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);
	while (1) {
		int val = -1;
		int ok = queue_get(q, &val);
        pthread_testcancel();
		if (!ok)
			continue;

		if (expected != val)
			printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

		expected = val + 1;
	}

	return NULL;
}

void *writer(void *arg) {
	int i = 0;
	queue_t *q = (queue_t *)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(2);

	while (1) {
//        usleep(1);
        pthread_testcancel();
		int ok = queue_add(q, i);
		if (!ok)
			continue;
		i++;
	}
	return NULL;
}

int main() {
	pthread_t write, read;
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(1000000);

	err = pthread_create(&read, NULL, reader, q);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

    //sched_yield();

	err = pthread_create(&write, NULL, writer, q);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

    sleep(10);

    struct rusage usage;
    pthread_cancel(read);
    pthread_cancel(write);
    pthread_join(read, NULL);
    pthread_join(write, NULL);
    getrusage(RUSAGE_SELF, &usage);
    clock_t user_usage = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
    clock_t system_usage = usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000;
    printf("user time: %ld [ms], system time %ld [ms]\n", user_usage, system_usage);
    printf("last stats: ");
    queue_print_stats(q);
	pthread_exit(NULL);

	return 0;
}
