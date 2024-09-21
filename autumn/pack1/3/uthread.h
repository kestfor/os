#ifndef UTHREAD_H
#define UTHREAD_H


typedef void *(*start_routine_t) (void*);
typedef struct uthread uthread;
typedef uthread* uthread_t;


int uthread_create(uthread_t *thread, start_routine_t start_routine, void *args);
void schedule(void);
void start_scheduling(void);

#endif //UTHREAD_H
