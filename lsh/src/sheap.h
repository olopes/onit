#ifndef __SHEAP_H__
#define __SHEAP_H__

#include "sexpr.h"

struct sheap {
    size_t capacity;
    size_t size;
    struct sexpression ** data;
};

#define SHEAP_OK 0
#define SHEAP_FULL 1
#define SHEAP_EXISTS 2
#define SHEAP_NULL 3
#define SHEAP_ERROR 4


/**
 * create new S-Heap 
 */
extern struct sheap * 
new_sheap();

/**
 * create new S-Heap with specified capacity
 */
extern struct sheap * 
new_sheap_capacity(size_t initial_capacity);

/**
 * Release S-Heap
 */
extern void 
free_sheap(struct sheap * heap);

/**
 * Return 1 if S-Heap is full
 */
extern int
sheap_is_full(struct sheap * heap);

/**
 * Returns the S-Heap internal capacity
 */
extern size_t
sheap_capacity(struct sheap * heap);

/**
 * Returns the number of elements in the S-Heap 
 */
extern size_t
sheap_size(struct sheap * heap);

/**
 * Insert element into S-Heap
 */
extern int 
sheap_insert(struct sheap * heap, struct sexpression * sexpr);

/**
 * Visit elements in the Heap
 */
extern int
sheap_visit(struct sheap * heap, void * param, void (*callback)(void * param, struct sexpression * value));

/**
 * Heap sort :-)
 */
extern int 
sheap_sort(struct sexpression * sexpr, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b));


#endif
