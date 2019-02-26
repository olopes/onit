#ifdef _INCLUDE_SCTX_C_


int enter_namespace(void * sctx) {
    struct sctx * env = (struct sctx *) sctx;
    struct shash_table * new_namespace;
    new_namespace = (struct shash_table *) malloc(sizeof(struct shash_table));
    if(new_namespace == NULL) {
        return 1;
    }
    memset(new_namespace, 0, sizeof(struct shash_table));
    sexpr_push(&env->namespaces, new_namespace);
    return 0;
}

int leave_namespace(void * sctx) {
    struct sctx * env = (struct sctx *) sctx;
    struct shash_table * old_namespace;
    old_namespace = (struct shash_table *) sexpr_pop(&env->namespaces);
    shash_free(old_namespace);
    free(old_namespace);
    return 0;
}

struct sexpression * lookup_name(void * sctx, struct svalue * name) {
    struct sctx * env = (struct sctx *) sctx;
    struct sexpression * namestack;
    struct shash_table * namespace;
    struct sexpression * value;
    
    value = shash_search(&env->primitives, name);
    
    if(value != NULL) {
        return value;
    }
    
    namestack = env->namespaces;
    
    while(namestack) {
        namespace = sexpr_car(namestack);
        namestack = sexpr_cdr(namestack);
        if(namespace == NULL) continue;
        value = shash_search(namespace, name);
        if(value != NULL) {
            return value;
        }
    }
    
    return NULL;
}

int register_value(void * sctx, struct svalue * name, struct sexpression * value) {
    struct sctx * env = (struct sctx *) sctx;
    struct sexpression * namestack;
    struct shash_table * namespace;
    
    namespace = (struct shash_table *) sexpr_peek(&env->namespaces);
    if(namespace == NULL) {
        return 1;
    }
    
    return shash_insert(namespace, name, value);
}


struct sexpression * get_reference(void * sctx, struct svalue * name, struct sexpression * obj) {
    struct sexpression * reference;
    
    reference = lookup_name(sctx, name);
    if(reference == NULL) {
        reference = new_reference(sctx, name, obj);
        register_value(sctx, name, reference);
    }
    
    return reference;
}

static struct sexpression * new_reference(struct sctx * sctx, struct svalue * name, struct sexpression * obj) {
    struct sexpression * reference;
    
    if(sctx->heap_load == sctx->heap_load) {
        recycle(sctx);
    }
    
    sctx->heap[sctx->heap_load] = obj;
    
    return obj;
}

static void recicle(struct sctx * sctx) {
    
    /* visit all nodes reachable from the namespaces */
    sctx->visit = !sctx->visit;
    
    visit_namespaces(sctx);
        
    free_unvisited_references(sctx);
    
    /* defreag heap */
    qsort(sctx->heap, sctx->heap_size, sizeof(struct sreference), sref_comparator);
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(sctx);
    
}

static void visit_namespaces(struct sctx * sctx) {
    struct sexpression * ns;
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
        shash_visit(namespace, visit_namespace_references);
        visit_namespace_elements(sexpr_car(ns), sctx->visit);
        ns = sexpr_cdr(ns);
    }
}

static void free_unvisited_references(struct sctx * sctx) {
    struct sexpression * ns;
    struct sexpression * values;
    struct sreference * heap;
    size_t i;
    
    for(i = 0, heap = sctx->heap; i < sctx->heap_size; i++, heap++) {
        if(heap->name != NULL && heap->visit != sctx->visit) {
            release_reference(heap);
            sctx->heap_load--;
        }
    }
}

static void grow_heap_if_necessary(struct sctx * sctx) {
    struct sexpression * ns;
    struct sexpression * values;
    struct sreference * heap;
    size_t new_size;
    
    if(sctx->heap_size < MAX_OBJ && sctx->heap_load*2 > sctx->heap_size) {
        new_size = sctx->heap_size << 1;
        heap = (struct sreference *) realloc(sctx->heap, sizeof(struct sreference)*new_size);
        if(heap != NULL) {
            memset(heap, 0, sizeof(struct sreference)*(new_size - sctx->heap_load));
            sctx->heap_size = new_size;
            sctx->heap = heap;
        }
    }
}

static void visit_namespace_references(struct svalue * key, struct sreference * reference) {
    if(namespace == NULL) {
        return;
    }
    
    
}

static void release_reference(struct sreference ** heap) {
    if(heap->name == NULL) {
        return;
    }
    sexpr_free_object(*heap);
    *heap = NULL;
}

static int sref_comparator(void * a, void * b) {
    unsigned long aa = (unsigned long)a;
    unsigned long bb = (unsigned long)b;
    return bb-aa;
}
    
#endif
