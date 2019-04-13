#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "shash.h"
#include "aa_tree.h"
#include "sctx.h"

#ifdef UNIT_TESTING
#include <signal.h>
#define heap_sanity_check(a,b) _heap_sanity_check(a,b)
#else
#define heap_sanity_check(a,b) ;
#endif

static int load_array_to_sexpr(struct sctx * sctx, struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void destroy_primitive_references_cb (void * sctx, struct sexpression * name, void * primitive_ptr);
static void free_heap_contents (struct mem_heap * heap);
static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void _heap_sanity_check(struct mem_heap * heap, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void prepare_visit(struct mem_heap * heap);
static void visit_namespaces(struct sctx * sctx);
static void free_unvisited_references(struct mem_heap * heap);
static void grow_heap_if_necessary(struct mem_heap * heap);
static inline int heap_should_grow(struct mem_heap * heap);
static void mark_reachable_references_cb (void * sctx_ptr, struct sexpression * key, void * reference);


/* some static/constant names */
static struct sexpression * key_true;

static struct sexpression * key_false;

#define ARGUMENTS_SYMBOL_NAME L"#args"
#define ARGUMENTS_SYMBOL_NAME_LENGTH 5
#define ENVIRONMENT_SYMBOL_NAME L"#end"
#define ENVIRONMENT_SYMBOL_NAME_LENGTH 4

struct sctx * 
create_new_sctx(char **argv, char **envp) {
    struct sctx * sctx;
    struct sexpression ** heap;
    struct sexpression * arg_key;
    struct sexpression * arg_list;
    struct sexpression * env_key;
    struct sexpression * env_list;
    
    
    
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
        .namespaces = NULL,
        .heap = (struct mem_heap) {
            .visit = 0,
            .size = HEAP_MIN_SIZE,
            .load = 0,
            .data = heap,
        },
        .namespace_destructor = NULL,
        .init_complete = 0,
    };
    
    
    /* setup locale? or assume already configured? */
    
    load_primitives(sctx);
    
    /* create two references */
    enter_namespace(sctx);
    /* register args and env. forget about primitives*/

    /* TODO extract function  ? */
    
    arg_key = alloc_new_value(sctx, ARGUMENTS_SYMBOL_NAME, ARGUMENTS_SYMBOL_NAME_LENGTH);
    load_array_to_sexpr(sctx, &arg_list, argv);
    env_key = alloc_new_value(sctx, ENVIRONMENT_SYMBOL_NAME, ENVIRONMENT_SYMBOL_NAME_LENGTH);
    load_array_to_sexpr(sctx, &env_list, envp);
    
    register_value(sctx, arg_key, arg_list);
    register_value(sctx, env_key, env_list);
    
    sctx->init_complete = 1;
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
            return SCTX_INIT_ERROR;
        }
        value = alloc_new_value(sctx, wcstr, wcslen(wcstr));
        if(value == NULL) {
            free(wcstr);
            return SCTX_INIT_ERROR;
        }
        list = alloc_new_pair(sctx, value, list);
        if(list == NULL) {
            free(wcstr);
            return SCTX_INIT_ERROR;
        }
        free(wcstr);
        array++;
    }
    
    *list_ptr = sexpr_reverse(list);
    return SCTX_INIT_OK;
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
    struct primitive * TRUE_PRIMITIVE;
    struct primitive * FALSE_PRIMITIVE;
    key_true = sexpr_create_value(L"#t",2);
    key_false = sexpr_create_value(L"#f",2);
    
    TRUE_PRIMITIVE = malloc(sizeof(struct primitive));
    *TRUE_PRIMITIVE = (struct primitive) {
        .type = PRIMITIVE_SEXPRESSION,
        .value = {.sexpression = key_true},
        .destructor = NULL,
    };
    
    FALSE_PRIMITIVE = malloc(sizeof(struct primitive));
    *FALSE_PRIMITIVE = (struct primitive) {
        .type = PRIMITIVE_SEXPRESSION,
        .value = {.sexpression = NULL},
        .destructor = NULL,
    };
    
    register_primitive(sctx, key_true, TRUE_PRIMITIVE);
    register_primitive(sctx, key_false, FALSE_PRIMITIVE);
    
    /*
    ARGUMENTS_PRIMITIVE.value.sexpression = sctx->arguments;
    register_primitive(sctx, &key_args, &ARGUMENTS_PRIMITIVE);
    
    ENVIRONMENT_PRIMITIVE.value.sexpression = sctx->environment;
    register_primitive(sctx, &key_env, &ENVIRONMENT_PRIMITIVE);
    */
}

int register_primitive(struct sctx * sctx, struct sexpression * name, struct primitive * primitive) {
    
    if(sctx == NULL || name == NULL) {
        return 1;
    }
    
    if(shash_has_key(&sctx->primitives, name)) {
        return 2;
    }
    
    return shash_insert(&sctx->primitives, name, primitive);
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
    sexpr_free(name);
    free(primitive_ptr);
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

int register_value(struct sctx * sctx, struct sexpression * name, struct sexpression * value) {
    struct shash_table * namespace;
    
    namespace = (struct shash_table *) sexpr_peek(&sctx->namespaces);
    if(namespace == NULL) {
        return 1;
    }
    
    return shash_insert(namespace, name, value);
}


struct sexpression * alloc_new_pair(struct sctx * sctx, struct sexpression * car, struct sexpression * cdr) {
    struct sexpression * object = sexpr_cons(car, cdr);
    
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
    struct sexpression * object = sexpr_create_value(wcstr, len);
    
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
    
    heap_sanity_check(heap, obj);
    
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

static void _heap_sanity_check(struct mem_heap * heap, struct sexpression * obj) {
    size_t i;
    
    for(i = 0; i < heap->load; i++) {
        if(heap->data[i] == NULL) {
            fprintf(stderr, "Memory heap at position %lu is NULL", i);
            raise(SIGSEGV);
        }
        if(heap->data[i] == obj) {
            fprintf(stderr, "Memory heap at position %lu is NULL", i);
            raise(SIGSEGV);
        }
    }
}

static void recycle(struct sctx * sctx) {
    
    if(sctx->init_complete) {
        /* visit all nodes reachable from the namespaces */
        prepare_visit(&sctx->heap);
    
        visit_namespaces(sctx);
        
        free_unvisited_references(&sctx->heap);
    }
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(&sctx->heap);
    
}

static void prepare_visit(struct mem_heap * heap) {
    /* TODO I want more than just a toggle. Let it be like this for now */
    heap->visit = !heap->visit;
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
        shash_visit(namespace, &sctx->heap.visit, mark_reachable_references_cb );
        ns = sexpr_cdr(ns);
    }
}

static void free_unvisited_references(struct mem_heap * heap) {
    size_t i = 0;

    while(i < heap->load ) {
        if ( heap->data[i] == NULL || sexpr_marked ( heap->data[i], heap->visit ) ) {
            i++;
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
    int visit = *(int *) visitp;
    struct sexpression * reference = (struct sexpression *) refp;
    
    if(reference == NULL) {
        return;
    }
    
    /* both key and reference are reachable */
    sexpr_mark_reachable(key, visit);
    sexpr_mark_reachable(reference, visit);
    
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
    
    for(i = 0; i < heap->load; i++) {
        sexpr_free_object(heap->data[i]);
    }
    heap->load = 0;
}
