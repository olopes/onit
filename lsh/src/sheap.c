#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "sheap.h"
#include "sexpr.h"


/* heap implementation to be used by the stuff */

/*
comparator usa uintptr_t 

https://algorithms.tutorialhorizon.com/binary-min-max-heap/
https://www.cs.cmu.edu/~adamchik/15-121/lectures/Binary%20Heaps/heaps.html
https://medium.com/100-days-of-algorithms/day-86-binary-heap-8af7c1abc8d0
*/

#define INITIAL_CAPACITY 1024
static void percolate_up(struct sexpression ** heap, size_t position);
static int contains_value(struct sheap * heap, uintptr_t value, size_t pos);


/**
 * create new S-Heap 
 */
struct sheap * new_sheap() {
    return new_sheap_capacity(INITIAL_CAPACITY);
}    

/**
 * create new S-Heap 
 */
struct sheap * new_sheap_capacity(size_t initial_capacity) {
    struct sheap * heap;
    struct sexpression ** data;
    
    heap = (struct sheap*) malloc(sizeof(struct sheap));
    if(heap == NULL) {
        return NULL;
    }
    
    data = (struct sexpression **) malloc(sizeof(struct sexpression*)*initial_capacity);
    if(data == NULL) {
        free(heap);
        return NULL;
    }
    
    *heap = (struct sheap) {
        .capacity = initial_capacity,
        .size = 0,
        .data = data,
    };
    
    return heap;
}

/**
 * Release S-Heap
 */
void free_sheap(struct sheap * heap) {
    if(heap == NULL) {
        return;
    }
    
    if(heap->data != NULL) {
        free(heap->data);
    }
    free(heap);
}


/**
 * Return 1 if S-Heap is full
 */
int
sheap_is_full(struct sheap * heap) {
    return heap != NULL && heap->size >= heap->capacity;
}

/**
 * Returns the S-Heap internal capacity
 */
size_t
sheap_capacity(struct sheap * heap) {
     return heap == NULL ? 0 : heap->capacity;
}

/**
 * Returns the number of elements in the S-Heap 
 */
size_t
sheap_size(struct sheap * heap) {
     return heap == NULL ? 0 : heap->size;
}

/**
 * Insert element into S-Heap
 */
int 
sheap_insert(struct sheap * heap, struct sexpression * sexpr) {
    if(heap == NULL) {
        return SHEAP_ERROR;
    }
    
    if(sexpr == NULL) {
        return SHEAP_NULL;
    }
    
    if(sheap_is_full(heap)) {
        return SHEAP_FULL;
    }
    
    if(contains_value(heap, (uintptr_t)sexpr, 1)) {
        return SHEAP_EXISTS;
    }
    
    
    heap->data[heap->size] = sexpr;
    heap->size++;
    
    percolate_up(heap->data-1, heap->size);
    
    
    return SHEAP_OK;
}

static int contains_value(struct sheap * heap, uintptr_t value, size_t pos) {
    uintptr_t node_value;
    
    if(pos > heap->size) {
        return 0;
    }
    
    node_value = (uintptr_t) heap->data[pos];
    if(node_value == value) {
        return 1;
    }
    
    /* max heap property */
    if(node_value < value) {
        return 0;
    }
    
    int sub_pos = pos << 1;
    return contains_value(heap, value, sub_pos) || contains_value(heap, value, sub_pos+1);
}

static void percolate_up(struct sexpression ** data, size_t position) {
    struct sexpression * tmp;
    size_t i = position;
    size_t parent = position >> 1;
    
    while (i > 1 && ((uintptr_t)data[i]) > ((uintptr_t)data[parent])) {
        tmp = data[parent];
        data[parent] = data[i];
        data[i] = tmp;

        i = parent;
        parent = i >> 1;
    }
}
    
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


/*


comparator usa uintptr_t 
int comparator (void * ptr_a, void * ptr_b) {
    uintptr_t a = ptr_a;
    uintptr_t b = ptr_b;
    
    if(a > b) {
        return 1;
    } else if(a == b) { 
        return 0;
    } else {
        return -1;
    }
}

*/
