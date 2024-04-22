#include "custom_malloc.h"
#include <sys/mman.h>
#include <inttypes.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

// 3 types of block
// 1 - used block (header + payload)
// 2 - free block (size = 24) (header + prev_link + next_link)
// 3 - free block (size > 24) (header + space + size + prev_link + next_link)

//addresses aligned for 8 bytes last 3 bits is 0, can use them as flag to read free blocks backwards

static size_t HEAP_SIZE = 1024 * 1024;

typedef struct header_t {
    uint last       :1;
    uint used       :1;
    uint prev_used  :1;
    uint size;
} header_t;

static int class(uint size) {
    if (size < 64) {
        return (int) (size - 8) >> 2;
    } else if (size < 256) {
        return 14;
    } else {
        return 15;
    }
}

//lists represented linked list of available blocks of memory
static header_t *lists[16] = {NULL};

//mapped pages
static header_t *head = NULL;

static header_t *next_header(header_t *header) {
    if (header->last) {
        return NULL;
    } else {
        return (void *) header + header->size + sizeof(header_t);
    }
}

static void *init_heap() {
    void *addr = mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (addr == (void *) - 1) {
        return NULL;
    } else {
        header_t *tmp = (void *) (addr);
        tmp->size = HEAP_SIZE - sizeof(header_t);
        tmp->last = true;
        tmp->prev_used = true;
        tmp->used = false;
        return addr;
    }
}

static header_t *previous_header_if_free(header_t *curr_header) {
    if (curr_header->prev_used) {
        return NULL;
    }
    uint64_t *footer = (void *) curr_header;
    uint64_t prev_block_size;
    if (footer[-1] & 1) {
        prev_block_size = 16;
    } else {
        prev_block_size = footer[-3];
    }
    return (void *) curr_header - prev_block_size - sizeof(header_t);
}

static header_t *split(header_t *curr_header, size_t size) {
    size_t extra_size = curr_header->size - size;
    if (extra_size < sizeof(header_t) + 16) {
        return NULL;
    }
    header_t *new_header = (void *) curr_header + size + sizeof(header_t);
    new_header->size = extra_size - sizeof(header_t);
    new_header->prev_used = curr_header->prev_used;
    new_header->last = curr_header->last;
    new_header->used = false;

    header_t *after_new = next_header(new_header);
    if (after_new != NULL) {
        after_new->prev_used = false;
    }

    curr_header->last = false;
    curr_header->size = size;
    return new_header;
}

static void merge(header_t *left, header_t *right) {
    left->last = right->last;
    left->size += sizeof(header_t) + right->size;

    header_t *next = next_header(right);
    if (next != NULL) {
        next->prev_used = left->used;
    }
}

//from free block
static header_t *next_link(header_t *header) {
    uint64_t *footer = (void *) header + header->size + sizeof(header_t);
    return (header_t *) (footer[-1] & ~7);
}

//from free block
static header_t *previous_link(header_t *header) {
    uint64_t *footer = (void *) header + header->size + sizeof(header_t);
    return (header_t *) (footer[-2]);
}

//set free block footer (2 or 3 type of block)
static void set_footer(header_t *header, header_t *prev_link, header_t *next_link) {
    uint64_t *footer = (void *) header + sizeof(header_t) + header->size;
    if (header->size > 16) {
        footer[-3] = (uint64_t) header->size;
        footer[-2] = (uint64_t) prev_link;
        footer[-1] = (uint64_t) next_link;
    } else {
        footer[-1] = (uint64_t) next_link | 1;
        footer[-2] = (uint64_t) prev_link;
    }
}

static header_t *best_fit(header_t *list, size_t size) {
    uint best_size = 0x77777777;
    header_t *best_match = NULL;
    for (header_t *curr = list; curr != NULL; curr = next_link(curr)) {
        if (curr->size >= size && best_size > curr->size) {
            best_size = curr->size;
            best_match = curr;
        }
    }
    return best_match;
}

static void remove_link(header_t *block, header_t *l[]) {
    uint cl = class(block->size);
    header_t *prev = previous_link(block);
    header_t *next = next_link(block);

    if (prev != NULL) {
        set_footer(prev, previous_link(prev), next);
    }

    if (next != NULL) {
        set_footer(next, prev, next_link(next));
    }

    if (l[cl] == block) {
        l[cl] = next;
    }
}

static void push_front(header_t *block, header_t *l[]) {
    uint cl = class(block->size);
    header_t *prev = NULL;
    header_t *next = l[cl];
    set_footer(block, prev, next);

    if (next != NULL) {
        set_footer(next, block, next_link(next));
    }

    l[cl] = block;
}

void *my_malloc(size_t size) {

    if (head == NULL) {
        head = init_heap();
        if (head == NULL) {
            return NULL;
        }
        push_front(head, lists);
    }

    //alignment
    if (size < 8) {
        size = 8;
    } else {
        size = (size + 7) & (~7);
    }

    header_t *block = NULL;

    for (uint cl = class(size); cl < 16 && block == NULL; cl++) {
        if (cl < 14) {
            block = lists[cl];
        } else {
            block = best_fit(lists[cl], size);
        }
    }

    if (block == NULL) {
        return NULL;
    }

    remove_link(block, lists);

    header_t *next = split(block, size);
    if (next != NULL) {
        push_front(next, lists);
        next->prev_used = true;
    }

    block->used = true;

    return (void *) block + sizeof(header_t);
}

static bool is_valid(header_t *header) {
    if (!(*(uint64_t *) header & ~7)) {
        return false;
    }

    if (!header->used) {
        return false;
    }

    if (header->last && next_link(header) != NULL) {
        return false;
    }

    return true;
}

void my_free(void *addr) {
    header_t *header = addr - sizeof(header_t);

    if (!is_valid(header)) {
        exit(SIGSEGV);
    }

    header_t *prev = previous_header_if_free(header);
    header_t *next = next_header(header);
    header->used = false;

    if (next != NULL) {
        next->prev_used = false;
        if (!next->used) {
            remove_link(next, lists);
            merge(header, next);
        }
    }

    if (prev != NULL && !prev->used) {
        remove_link(prev, lists);
        merge(prev, header);
        header = prev;
    }

    push_front(header, lists);
}
