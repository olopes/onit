#include <stddef.h>
#include "sctx_privates.h"

static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void visit_namespaces(struct sctx * sctx);
static void free_unvisited_references(struct sctx * sctx);
static void grow_heap_if_necessary(struct sctx * sctx);
static inline int heap_should_grow(struct sctx * sctx);
static void mark_reachable_references(void * ctx, struct sexpression * key, void * reference);
static int sref_comparator(const void * a, const void * b);



int enter_namespace(void * sctx_ptr) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    struct shash_table * new_namespace;
    new_namespace = (struct shash_table *) malloc(sizeof(struct shash_table));
    if(new_namespace == NULL) {
        return 1;
    }
    memset(new_namespace, 0, sizeof(struct shash_table));
    sexpr_push(&sctx->namespaces, new_namespace);
    return 0;
}

int leave_namespace(void * sctx_ptr) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    struct shash_table * old_namespace;
    old_namespace = (struct shash_table *) sexpr_pop(&sctx->namespaces);
    shash_free(old_namespace);
    free(old_namespace);
    return 0;
}

struct sexpression * lookup_name(void * sctx_ptr, struct sexpression * name) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    struct sexpression * namestack;
    struct shash_table * namespace;
    struct sexpression * value;
    
    value = shash_search(&sctx->primitives, name);
    
    if(value != NULL) {
        return value;
    }
    
    namestack = sctx->namespaces;
    
    while(namestack) {
        namespace = (struct shash_table *) sexpr_car(namestack);
        if(namespace == NULL) continue;
        value = shash_search(namespace, name);
        if(value != NULL) {
            return value;
        }
        namestack = sexpr_cdr(namestack);
    }
    
    return NULL;
}

int register_value(void * sctx_ptr, struct sexpression * name, struct sexpression * value) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    struct shash_table * namespace;
    
    namespace = (struct shash_table *) sexpr_peek(&sctx->namespaces);
    if(namespace == NULL) {
        return 1;
    }
    
    return shash_insert(namespace, name, value);
}


struct sexpression * alloc_new_pair(void * sctx, struct sexpression * car, struct sexpression * cdr) {
    struct sexpression * object = sexpr_cons(car, cdr);
    
    if(object == NULL) {
        return NULL;
    }
    
    if(record_new_object((struct sctx *) sctx, object)) {
        /* mem full? error? */
        sexpr_free_object(object);
        object = NULL;
    }
    
    return object;
}
    
struct sexpression * alloc_new_value(void * sctx, wchar_t * wcstr, size_t len) {
    /* future improvement: string cache */
    struct sexpression * object = sexpr_create_value(wcstr, len);
    
    if(object == NULL) {
        return NULL;
    }
    
    if(record_new_object((struct sctx *) sctx, object)) {
        /* mem full? error? */
        sexpr_free_object(object);
        object = NULL;
    }
    
    return object;
}


static int record_new_object(struct sctx * sctx, struct sexpression * obj) {
    
    if(sctx->heap_load >= sctx->heap_size) {
        recycle(sctx);
        
        /* FIXME handle errors ? */
        if(sctx->heap_load >= sctx->heap_size) {
            return 1;
        }
    }
    
    sctx->heap[sctx->heap_load] = obj;
    sctx->heap_load++;
    
    return 0;
}

static void recycle(struct sctx * sctx) {
    
    /* visit all nodes reachable from the namespaces */
    sctx->visit = !sctx->visit;
    
    visit_namespaces(sctx);
        
    free_unvisited_references(sctx);
    
    /* defreag heap */
    qsort(sctx->heap, sctx->heap_size, sizeof(struct sexpression *), sref_comparator);
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(sctx);
    
}

static void visit_namespaces(struct sctx * sctx) {
    struct sexpression * ns;
    struct shash_table * namespace;
    ns = sctx->namespaces;
    
    /*
     * Algorithm:
     * for each namespace
     *   for each reference in namespace
     *     mark reference
     *     if(reference is list and reference is not visited)
     *        visit car
     *        visit cdr
     */
    
    while(ns != NULL) {
        namespace = (struct shash_table *) sexpr_car(ns);
        shash_visit(namespace, sctx, mark_reachable_references);
        ns = sexpr_cdr(ns);
    }
}

static void free_unvisited_references(struct sctx * sctx) {
    struct sexpression * ns;
    struct sexpression * values;
    struct sexpression ** heap;
    size_t i;
    
    for(i = 0, heap = sctx->heap; i < sctx->heap_size; i++, heap++) {
        if(*heap != NULL && !sexpr_marked(*heap, sctx->visit)) {
            sexpr_free_object(*heap);
            *heap = NULL;
            sctx->heap_load--;
        }
    }
}


static void grow_heap_if_necessary(struct sctx * sctx) {
    struct sexpression * ns;
    struct sexpression * values;
    struct sexpression ** heap;
    size_t new_size;
    
    if(heap_should_grow(sctx)) {
        new_size = sctx->heap_size << 1;
        heap = (struct sexpression **) realloc(sctx->heap, sizeof(struct sexpression *)*new_size);
        if(heap != NULL) {
            memset(heap, 0, sizeof(struct sexpression *) * (new_size - sctx->heap_load));
            sctx->heap_size = new_size;
            sctx->heap = heap;
        }
    }
}

static inline int heap_should_grow(struct sctx * sctx) {
    return (sctx->heap_size < HEAP_MIN_SIZE && sctx->heap_load*2 > sctx->heap_size);
}

static void mark_reachable_references(void * sctx_ptr, struct sexpression * key, void * reference) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(reference == NULL) {
        return;
    }
    
    /* utility specific to sexpr to reach all nodes */
    sexpr_mark_reachable((struct sexpression *) reference, sctx->visit);
    
}

static int sref_comparator(const void * a, const void * b) {
    unsigned long aa = (unsigned long)a;
    unsigned long bb = (unsigned long)b;
    return bb-aa;
}

void
sctx_gc(void * sctx_ptr) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(sctx == NULL) {
        return;
    }
    
    recycle(sctx);
    
}

