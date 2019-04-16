#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <signal.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "shash.h"
#include "aa_tree.h"
#include "sctx.h"

static int load_array_to_sexpr(struct sctx * sctx, struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void destroy_primitive_references_cb (void * sctx, struct sexpression * name, void * primitive_ptr);
static void release_primitive(struct sctx * sctx, struct primitive * primitive);
static void free_heap_contents (struct mem_heap * heap);
static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void visit_namespaces(struct sctx * sctx);
static void free_unvisited_references(struct mem_heap * heap);
static void grow_heap_if_necessary(struct mem_heap * heap);
static inline int heap_should_grow(struct mem_heap * heap);
static void mark_reachable_references_cb (void * sctx_ptr, struct sexpression * key, void * reference);
static int 
create_reference(struct shash_table * table, struct sexpression * name, struct mem_reference * reference);

#ifdef UNIT_TESTING
static void heap_sanity_check(struct mem_heap * heap, struct sexpression * obj);
static void print_heap_contents(struct mem_heap * heap);
#endif

/* some static/constant names */
#define ARGUMENTS_SYMBOL_NAME L"#args"
#define ARGUMENTS_SYMBOL_NAME_LENGTH 5
#define ENVIRONMENT_SYMBOL_NAME L"#end"
#define ENVIRONMENT_SYMBOL_NAME_LENGTH 4

struct sctx * 
create_new_sctx(char **argv, char **envp) {
    struct sctx * sctx;
    struct sexpression ** heap;
    
    sctx = (struct sctx *) malloc(sizeof(struct sctx));
    if(sctx == NULL) {
        return NULL;
    }
    
    heap = (struct sexpression **) malloc(sizeof(struct sexpression *) * HEAP_MIN_SIZE);
    if(heap == NULL) {
        free(sctx);
        return NULL;
    }
    
    memset(heap, 0, sizeof(struct sexpression *) * HEAP_MIN_SIZE);
    
    *sctx = (struct sctx) {
        .primitives = {
            .size=0,
            .load=0,
            .table=NULL,
        },
        .global = NULL,
        .namespaces = NULL,
        .heap = (struct mem_heap) {
            .visit = 0,
            .size = HEAP_MIN_SIZE,
            .load = 0,
            .data = heap,
        },
        .namespace_destructor = NULL,
    };
    
    
    /* setup locale? or assume already configured? */
    
    /* create and keep reference for global namespace */
    enter_namespace(sctx);
    sctx->global = (struct shash_table *) sexpr_peek(&sctx->namespaces);
    
    load_primitives(sctx);
    
    /* create two references */
    /* register args and env. forget about primitives*/

    /* TODO extract function  ? */
    struct mem_reference reference;
    create_global_reference(sctx, ARGUMENTS_SYMBOL_NAME, ARGUMENTS_SYMBOL_NAME_LENGTH, &reference);
    load_array_to_sexpr(sctx, (struct sexpression **) reference.value, argv);
    create_global_reference(sctx, ENVIRONMENT_SYMBOL_NAME, ENVIRONMENT_SYMBOL_NAME_LENGTH, &reference);
    load_array_to_sexpr(sctx, (struct sexpression **) reference.value, envp);
    
    return sctx;
}



static int load_array_to_sexpr(struct sctx * sctx, struct sexpression ** list_ptr, char ** array) {
    wchar_t * wcstr;
    struct sexpression * list;
    struct sexpression * value;
    list = NULL;
    
    while(*array) {
        wcstr = convert_to_wcstr(*array);        
        if(wcstr == NULL) {
            return SCTX_ERROR;
        }
        value = alloc_new_value(sctx, wcstr, wcslen(wcstr));
        if(value == NULL) {
            free(wcstr);
            return SCTX_ERROR;
        }
        list = alloc_new_pair(sctx, value, list);
        if(list == NULL) {
            free(wcstr);
            return SCTX_ERROR;
        }
        free(wcstr);
        array++;
    }
    
    *list_ptr = sexpr_reverse(list);
    return SCTX_OK;
}    

static wchar_t * convert_to_wcstr(const char * src) {
    wchar_t * wcstr;
    size_t len;
    size_t conv;
    
    len = mbstowcs(NULL, src, 0);
    if(len == -1) {
        return NULL;
    }
    wcstr = (wchar_t *) malloc(sizeof(wchar_t)*(len+1));
    if(wcstr == NULL) {
        return NULL;
    }
    conv = mbstowcs(wcstr, src, len+1);
    if(conv != len) {
        free(wcstr);
        return NULL;
    }
    return wcstr;
}

static void load_primitives(struct sctx * sctx) {
    struct mem_reference reference;
    struct primitive * TRUE_PRIMITIVE;
    struct primitive * FALSE_PRIMITIVE;
    
    TRUE_PRIMITIVE = malloc(sizeof(struct primitive));
    *TRUE_PRIMITIVE = (struct primitive) {
        .type = PRIMITIVE_SEXPRESSION,
        .value = {.sexpression = (struct sexpression*)1},
        .destructor = NULL,
    };
    
    FALSE_PRIMITIVE = malloc(sizeof(struct primitive));
    *FALSE_PRIMITIVE = (struct primitive) {
        .type = PRIMITIVE_SEXPRESSION,
        .value = {.sexpression = NULL},
        .destructor = NULL,
    };
    
    create_primitive_reference(sctx, L"#t",2, &reference);
    *reference.value = TRUE_PRIMITIVE;
    create_primitive_reference(sctx, L"#f",2, &reference);
    *reference.value = FALSE_PRIMITIVE;
    
}

int create_primitive_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_value(sctx, wcstr, len);
    
    return create_reference(&sctx->primitives, name, reference);
}

int create_global_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_value(sctx, wcstr, len);
    
    return create_reference(sctx->global, name, reference);
}

int create_stack_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    struct shash_table * namespace;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_value(sctx, wcstr, len);
    
    namespace = (struct shash_table *) sexpr_peek(&sctx->namespaces);
    if(namespace == NULL) {
        return SCTX_ERROR;
    }
    
    return create_reference(namespace, name, reference);
}

static int 
create_reference(struct shash_table * table, struct sexpression * name, struct mem_reference * reference) {
    struct shash_entry * entry;
    
    if(!shash_has_key(table, name)) {
        shash_insert(table, name, NULL);
    }
    
    entry = shash_get_entry(table, name);
    
    *reference = (struct mem_reference) {
        .key = &entry->key,
        .value = &entry->value,
    };
    
    return SCTX_OK;
}



void 
release_sctx(struct sctx * sctx) {
    
    if(sctx == NULL) {
        return;
    }
    
    
    /* TODO release remaining primitives before releasing the hashtable */
    shash_visit(&sctx->primitives, sctx, destroy_primitive_references_cb );
    shash_free(&sctx->primitives);

    leave_namespace(sctx);
    /* sctx_gc(sctx); */
    
    free_heap_contents(&sctx->heap);
    
    free(sctx->heap.data);
    free(sctx);
}

static void destroy_primitive_references_cb (void * sctx, struct sexpression * name, void * primitive_ptr) {
    /* name will be garbage collected */
    release_primitive((struct sctx *) sctx, (struct primitive *)primitive_ptr);
}

static void release_primitive(struct sctx * sctx, struct primitive * primitive) {
    if(primitive->destructor != NULL) {
        primitive->destructor(sctx, primitive);
    } else {
        free(primitive);
    }
}
    


int enter_namespace(struct sctx * sctx) {
    struct shash_table * new_namespace;
    new_namespace = (struct shash_table *) malloc(sizeof(struct shash_table));
    if(new_namespace == NULL) {
        return 1;
    }
    memset(new_namespace, 0, sizeof(struct shash_table));
    sexpr_push(&sctx->namespaces, new_namespace);
    return 0;
}

int leave_namespace(struct sctx * sctx) {
    struct shash_table * old_namespace;
    old_namespace = (struct shash_table *) sexpr_pop(&sctx->namespaces);
    shash_free(old_namespace);
    free(old_namespace);
    return 0;
}

struct sexpression * lookup_name(struct sctx * sctx, struct sexpression * name) {
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

struct sexpression * alloc_new_pair(struct sctx * sctx, struct sexpression * car, struct sexpression * cdr) {
    struct sexpression * object;
    
    object = sexpr_cons(car, cdr);
    
    if(object == NULL) {
        return NULL;
    }
    
    if(record_new_object(sctx, object)) {
        /* mem full? error? */
        sexpr_free_object(object);
        object = NULL;
    }
    
    return object;
}
    
struct sexpression * alloc_new_value(struct sctx * sctx, wchar_t * wcstr, size_t len) {
    /* future improvement: string cache */
    struct sexpression * object;
    
    object = sexpr_create_value(wcstr, len);
    
    if(object == NULL) {
        return NULL;
    }
    
    if(record_new_object(sctx, object)) {
        /* mem full? error? */
        sexpr_free_object(object);
        object = NULL;
    }
    
    return object;
}


static int record_new_object(struct sctx * sctx, struct sexpression * obj) {
    struct mem_heap * heap = &sctx->heap;
    
#ifdef UNIT_TESTING
    heap_sanity_check(heap, obj);
#endif
    
    if(heap->load >= heap->size ) {
        recycle(sctx);
        
        /* FIXME handle errors ? */
        if(heap->load >= heap->size ) {
            return 1;
        }
    }
    
    heap->data[heap->load] = obj;
    heap->load++;
    
    return 0;
}

#ifdef UNIT_TESTING
static void heap_sanity_check(struct mem_heap * heap, struct sexpression * obj) {
    size_t i;
    
    for(i = 0; i < heap->load; i++) {
        if(heap->data[i] == NULL) {
            fprintf(stderr, "ERROR: heap_sanity_check Memory heap at position %lu is NULL\n", (unsigned long) i);
            raise(SIGSEGV);
        }
        if(heap->data[i] == obj) {
            fprintf(stderr, "ERROR: heap_sanity_check Reference %p already in the heap!\n", (void*)obj);
            raise(SIGSEGV);
        }
    }
}
#endif

static void recycle(struct sctx * sctx) {
    
    /* visit all nodes reachable from the namespaces */
    visit_namespaces(sctx);
    free_unvisited_references(&sctx->heap);
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(&sctx->heap);
    
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
        shash_visit(namespace, NULL, mark_reachable_references_cb );
        ns = sexpr_cdr(ns);
    }
}

static void free_unvisited_references(struct mem_heap * heap) {
    size_t i = 0;

    while(i < heap->load ) {
        if ( heap->data[i] == NULL || sexpr_is_marked ( heap->data[i], 1 ) ) {
            i++;
            sexpr_set_mark(heap->data[i], 0);
        } else {
            sexpr_free_object ( heap->data[i] );
            heap->data[i] = heap->data[heap->load];
            heap->data[heap->load] = NULL;
            heap->load--;
        }
    }
}


static void grow_heap_if_necessary(struct mem_heap * heap) {
    struct sexpression ** data;
    size_t new_size;
    
    if(heap_should_grow(heap)) {
        new_size = heap->size << 1;
        data = (struct sexpression **) realloc(heap->data, sizeof(struct sexpression *)*new_size);
        if(data != NULL) {
            memset(data, 0, sizeof(struct sexpression *) * (new_size - heap->load ));
            heap->size = new_size;
            heap->data = data;
        }
    }
}

static inline int heap_should_grow(struct mem_heap * heap) {
    return (heap->size < HEAP_MIN_SIZE && heap->load*2 > heap->size );
}

static void mark_reachable_references_cb (void * visitp, struct sexpression * key, void * refp) {
    struct sexpression * reference = (struct sexpression *) refp;
    
    if(reference == NULL) {
        return;
    }
    
    /* both key and reference are reachable */
    sexpr_mark_reachable(key, 1);
    sexpr_mark_reachable(reference, 1);
    
}

void
sctx_gc(struct sctx * sctx) {
    if(sctx == NULL) {
        return;
    }
    
    recycle(sctx);
    
}

static void
free_heap_contents (struct mem_heap * heap) {
    size_t i;
    struct sexpression ** data = heap->data;

#ifdef UNIT_TESTING
    print_heap_contents(heap);
#endif

    for(i = 0; i < heap->load; i++) {
        sexpr_free_object(data[i]);
    }
    heap->load = 0;
}

#ifdef UNIT_TESTING
static void print_heap_contents(struct mem_heap * heap) {
    size_t i;
    struct sexpression ** data = heap->data;
    struct sexpression * obj;
    FILE * const out = stdout;
    
    fprintf(out, "HEAP CONTENTS: [%lu/%lu]\n", (unsigned long) heap->load, (unsigned long) heap->size);
    for(i = 0; i < heap->size; i++) {
        obj = data[i];
        if(sexpr_is_value(obj)) {
            fprintf(out, "\tVALUE object in the heap: %p => \"%.*ls\"\n", (void*)obj, (int)obj->len, obj->data.value); 
        } else if(sexpr_is_cons(obj)) {
            fprintf(out, "\tCONS  object in the heap: %p => (%p . %p)\n", (void*)obj, (void*)obj->data.sexpr, (void*)obj->cdr.sexpr); 
        } else {
            fprintf(out, "\tFUNNY object in the heap: %p => type[%d]\n", (void*)obj, sexpr_type(obj)); 
        }        
    }
    fflush(out);
}
#endif
