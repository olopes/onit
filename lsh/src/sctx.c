#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "shash.h"
#include "aa_tree.h"
#include "sctx.h"


struct sctx {
    struct shash_table primitives;
    struct sexpression * namespaces;
    int visit;
    int heap_size;
    int heap_load;
    struct sexpression ** heap;
    void (*namespace_destructor)(struct sctx * ctx);
};

#define HEAP_MIN_SIZE 32
#define HEAP_MAX_SIZE 131072


static void load_array_to_sexpr(struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void destroy_primitive_references(void * sctx, struct sexpression * name, void * primitive_ptr);
static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void visit_namespaces(struct sctx * sctx);
static void free_unvisited_references(struct sctx * sctx);
static void grow_heap_if_necessary(struct sctx * sctx);
static inline int heap_should_grow(struct sctx * sctx);
static void mark_reachable_references(void * ctx, struct sexpression * key, void * reference);
static int sref_comparator(const void * a, const void * b);

/* some static/constant names */
static struct sexpression * key_true;

static struct sexpression * key_false;

void * 
init_environment(char **argv, char **envp) {
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
        .visit = 0,
        .heap_size = HEAP_MIN_SIZE,
        .heap_load = 4,
        .heap = heap,
        .namespace_destructor = NULL,
    };
    
    
    /* setup locale? or assume already configured? */
    
    load_primitives(sctx);
    
    /* create two references */
    enter_namespace(sctx);
    /* register args and env. forget about primitives*/

    /* TODO extract function  ? */
    
    arg_key = sexpr_create_value(L"#args", 5);
    load_array_to_sexpr(&arg_list, argv);
    env_key = sexpr_create_value(L"#env", 4);
    load_array_to_sexpr(&env_list, envp);
    
    register_value(sctx, arg_key, arg_list);
    register_value(sctx, env_key, env_list);
    
    /* store in the heap */
    heap[0] = arg_key;
    heap[1] = arg_list;
    heap[2] = env_key;
    heap[3] = env_list;

    
    return sctx;
}



static void load_array_to_sexpr(struct sexpression ** list_ptr, char ** array) {
    wchar_t * wcstr;
    struct sexpression * list;
    list = NULL;
    
    while(*array) {
        wcstr = convert_to_wcstr(*array);        
        list = sexpr_cons(sexpr_create_value(wcstr, wcslen(wcstr)), list);
        free(wcstr);
        array++;
    }
    
    *list_ptr = sexpr_reverse(list);
    
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

int register_primitive(void * sctx_ptr, struct sexpression * name, struct primitive * primitive) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(sctx == NULL || name == NULL) {
        return 1;
    }
    
    if(shash_has_key(&sctx->primitives, name)) {
        return 2;
    }
    
    return shash_insert(&sctx->primitives, name, primitive);
}



void 
release_environment(void * sctx_ptr) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(sctx == NULL) {
        return;
    }
    
    
    /* TODO release remaining primitives before releasing the hashtable */
    shash_visit(&sctx->primitives, sctx, destroy_primitive_references);
    shash_free(&sctx->primitives);

    leave_namespace(sctx);
    sctx_gc(sctx);
    
    free(sctx->heap);
    free(sctx);
}

static void destroy_primitive_references(void * sctx, struct sexpression * name, void * primitive_ptr) {
    sexpr_free(name);
    free(primitive_ptr);
}


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

