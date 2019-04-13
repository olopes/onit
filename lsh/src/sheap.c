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
static void 
heapify(struct sexpression ** sexpr, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b));
static void 
sift_down(struct sexpression ** sexpr, size_t start, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b));
static void 
swap_elements (struct sexpression ** sexpr, size_t a, size_t b);


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
    
    heap->data[heap->size] = sexpr;
    heap->size++;
    
    percolate_up(heap->data-1, heap->size);
    
    
    return SHEAP_OK;
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
void
sheap_visit(struct sheap * heap, void * param, void (*callback)(void * param, struct sexpression * value)) {
    size_t i;
    struct sexpression ** data;
    
    if(heap == NULL || callback == NULL) {
        return;
    }
    
    for(i = 0, data = heap->data; i < heap->size; i++, data++) {
        callback(param, *data);
    }
}
    

/**
 * Heap sort :-)
 * 
 * Reference implementation taken from Wikipedia
 */
int 
sheap_sort(struct sexpression ** sexpr, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b)) {
    size_t end;
    if(sexpr == NULL || comparator == NULL) {
        return SHEAP_ERROR;
    }
    
    if(size <= 1) {
        /* already sorted */
        return 0;
    }
    
    heapify(sexpr, size, comparator);
    end = size - 1;
    while(end > 0) {
        swap_elements(sexpr, 0, end);
        end--;
        sift_down(sexpr, 0, end, comparator);
    }
    
    
    return 0;
}

#define left_child(position) ((position)*2+1)

static void 
heapify(struct sexpression ** sexpr, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b)) {
    size_t start;
    size_t end;
    
    end = size - 1;
    start = left_child(end);
    
    while(start > 0) {
        sift_down(sexpr, start, end, comparator);
        start--;
    }
    sift_down(sexpr, start, end, comparator);
}

static void 
sift_down(struct sexpression ** sexpr, size_t start, size_t end, int (*comparator)(struct sexpression * a, struct sexpression * b)) {
    size_t root;
    size_t swap;
    size_t left;
    size_t right;
    
    root = start;
    left = left_child(root);
    
    while(left <= end) {
        right = left + 1;
        swap = root;
        
        if (comparator(sexpr[swap], sexpr[left]) < 0) {
            swap = left;
        }
        
        if(right <= end && comparator(sexpr[swap], sexpr[right]) < 0) {
            swap = right;
        }
        
        if(swap == root) {
            return;
        }
        
        swap_elements(sexpr, root, swap);

        root = swap;
        left = left_child(root);
    }
    
}

static void 
swap_elements(struct sexpression ** sexpr, size_t a, size_t b) {
    struct sexpression * tmp;
    
    tmp = sexpr[a];
    sexpr[a] = sexpr[b];
    sexpr[b] = tmp;
}

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
