#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "uthread.h"
#define PAGE_SIZE  4096
#define STACK_SIZE (PAGE_SIZE*8)

typedef struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
} list_node;


struct uthread {

    ucontext_t ctx;
    start_routine_t start_routine;
    void *args;
    list_node node;

};

static uthread main_thread;

static list_node *curr_head = NULL;
static list_node *curr_tail = NULL;
static list_node *curr = NULL;

static bool main_initialized = false;

void *create_stack(int stack_size) {
    void *stack;
    stack = mmap(NULL, stack_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return stack;
}

void remove_thread_link(list_node *node) {
    if (node == curr_head) {
        curr_head = node->next;
    }
    if (node == curr_tail) {
        curr_tail = node->prev;
    }

    ucontext_t prev = ((uthread *) node->prev->data)->ctx;
    ucontext_t next = ((uthread *) node->next->data)->ctx;
    if (&prev == &next) {
        next.uc_link = NULL;
    } else {
        next.uc_link = &prev;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;

}

int clean_up(uthread *th) {
    return munmap(th + sizeof(uthread) - STACK_SIZE, STACK_SIZE);
}

void thread_routine_wrapper(int higher_bits, int lower_bits) {
    int *int_casted = (int *) ((((long long) higher_bits) << 32) | (((long long) lower_bits) & 0xffffffff));
    uthread *thread = (uthread *) int_casted;
    void *res = thread->start_routine(thread->args);
    printf("thread finished\n");
    remove_thread_link(&thread->node);
    printf("thread link cleared\n");
    clean_up(thread);
}

int uthread_create(uthread_t *thread, start_routine_t start_routine, void *args) {

    if (!main_initialized) {
        main_initialized = true;
        main_thread.node.data = &main_thread;
        main_thread.node.prev = &main_thread.node;
        main_thread.node.next = &main_thread.node;
        curr_head = &main_thread.node;
        curr_tail = &main_thread.node;
        curr = curr_head;
    }

    void *stack = create_stack(STACK_SIZE);

    if (mprotect(stack + PAGE_SIZE, STACK_SIZE - PAGE_SIZE, PROT_READ | PROT_WRITE) != 0) {
        return -1;
    }

    uthread *new_thread = (stack + STACK_SIZE - sizeof(uthread));
    new_thread->start_routine = start_routine;
    new_thread->args = args;

    getcontext(&(new_thread->ctx));
    new_thread->ctx.uc_stack.ss_sp = stack;
    new_thread->ctx.uc_stack.ss_size = STACK_SIZE - sizeof(uthread);

    int higher, lower;

    int *int_casted = (int *) new_thread;

    lower = (int) (((long) int_casted) & 0xffffffff);
    higher = (int) ((((long) int_casted) >> 32) & 0xffffffff);

    list_node *node_addr = &new_thread->node;
    new_thread->node.data = new_thread;

    new_thread->node.next = curr_head;
    curr_head->prev = node_addr;

    new_thread->node.prev = curr_tail;
    curr_tail->next = node_addr;

    curr_tail = node_addr;
    new_thread->ctx.uc_link = &((uthread *) (node_addr->prev->data))->ctx;

    makecontext(&(new_thread->ctx), (void (*)()) thread_routine_wrapper, 2, higher, lower);

    *thread = new_thread;

    return 0;
}

void schedule(void) {
    ucontext_t *curr_ctx, *next_ctx;

    curr_ctx = &(((uthread *) curr->data)->ctx);
    next_ctx = &(((uthread *) (curr->next->data))->ctx);
    curr = curr->next;

    if (swapcontext(curr_ctx, next_ctx) == -1) {
        printf("swap context failed");
        exit(1);
    }

}

