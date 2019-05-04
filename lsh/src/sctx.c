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
#include "svisitor.h"

static int load_array_to_sexpr(struct sctx * sctx, struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void free_heap_contents (struct mem_heap * heap);
static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void free_unvisited_references(struct mem_heap * heap);
static void grow_heap_if_necessary(struct mem_heap * heap);
static inline int heap_should_grow(struct mem_heap * heap);
static void mark_reachable_references_cb (void * sctx_ptr, struct sexpression * key, void * reference);
static int 
create_reference(struct shash_table * table, struct sexpression * name, struct mem_reference * reference);
static void 
move_to_heap_visitor(struct sexpression * sexpr, struct scallback * callback);

#ifdef UNIT_TESTING
static void heap_sanity_check(struct mem_heap * heap, struct sexpression * obj);
static void print_heap_contents(struct mem_heap * heap, char * msg);
#endif

/* some static/constant names */
#define ARGUMENTS_SYMBOL_NAME L"#args"
#define ARGUMENTS_SYMBOL_NAME_LENGTH 5
#define ENVIRONMENT_SYMBOL_NAME L"#end"
#define ENVIRONMENT_SYMBOL_NAME_LENGTH 4


static void 
shash_namespace_destructor(void * shash);
static void 
shash_namespace_visit(void * shash);
static void 
shash_namespace_mark_reachable(void * shash, unsigned char mark);

static struct sprimitive shash_namespace_handler = {
    .destructor=shash_namespace_destructor,
    .print = NULL,
    .visit=shash_namespace_visit,
    .mark_reachable = shash_namespace_mark_reachable,
    .is_marked = NULL,
    .compare=NULL
};



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
        .protected_namespace = {
            .size=0,
            .load=0,
            .table=NULL,
        },
        .global_namespace = NULL,
        .namespaces = NULL,
        .in_load = NULL,
        .heap = (struct mem_heap) {
            .visit = 0,
            .size = HEAP_MIN_SIZE,
            .load = 0,
            .data = heap,
        }
    };
    
    
    /* setup locale? or assume already configured? */
    
    /* create and keep reference for global namespace */
    enter_namespace(sctx);
    sctx->global_namespace = (struct shash_table *) sexpr_primitive_ptr(sexpr_peek(&sctx->namespaces));
    
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
    
    if(array == NULL) {
        *list_ptr = NULL;
        return SCTX_OK;
    }
    
    while(*array) {
        wcstr = convert_to_wcstr(*array);        
        if(wcstr == NULL) {
            return SCTX_ERROR;
        }
        value = alloc_new_string(sctx, wcstr, wcslen(wcstr));
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

static void print_boolean_primitive(void * ptr);

static struct sprimitive boolean_primitive_handler = {
    .print=print_boolean_primitive
};

static wchar_t * T = L"#t";
static wchar_t * F = L"#f";

static void print_boolean_primitive(void * ptr) {
    wprintf(L"%ls", (wchar_t *) ptr);
}

static void load_primitives(struct sctx * sctx) {
    struct mem_reference reference;
    struct sexpression * TRUE_PRIMITIVE = alloc_new_primitive(sctx, T, &boolean_primitive_handler);
    struct sexpression * FALSE_PRIMITIVE = alloc_new_primitive(sctx, F, &boolean_primitive_handler);
    
    create_protected_reference (sctx, T, 2, &reference);
    *reference.value = TRUE_PRIMITIVE;
    create_protected_reference (sctx, F, 2, &reference);
    *reference.value = FALSE_PRIMITIVE;
    
}

int create_protected_reference (struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_symbol(sctx, wcstr, len);
    
    return create_reference(&sctx->protected_namespace, name, reference);
}

int create_global_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_symbol(sctx, wcstr, len);
    
    return create_reference(sctx->global_namespace, name, reference);
}

int create_stack_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference) {
    struct sexpression * name;
    struct sexpression * namespace;
    
    if(sctx == NULL || wcstr == NULL || len == 0 || reference == NULL) {
        return SCTX_ERROR;
    }
    
    name = alloc_new_symbol(sctx, wcstr, len);

    namespace = sexpr_peek(&sctx->namespaces);
    if(namespace == NULL) {
        return SCTX_ERROR;
    }
    
    return create_reference((struct shash_table *) sexpr_primitive_ptr(namespace), name, reference);
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
    
    
    shash_free(&sctx->protected_namespace );

    leave_namespace(sctx);
    /* sctx_gc(sctx); */
    
    free_heap_contents(&sctx->heap);
    
    free(sctx->heap.data);
    free(sctx);
}

int enter_namespace(struct sctx * sctx) {
    struct shash_table * new_namespace;
    struct sexpression * primitive;
    new_namespace = (struct shash_table *) malloc(sizeof(struct shash_table));
    if(new_namespace == NULL) {
        return SCTX_ERROR;
    }
    memset(new_namespace, 0, sizeof(struct shash_table));
    
    primitive = alloc_new_primitive(sctx, new_namespace, &shash_namespace_handler );
    if(primitive == NULL) {
        shash_free(new_namespace);
        return SCTX_ERROR;
    }
    
    sexpr_push(&sctx->namespaces, primitive);
    return SCTX_OK;
}

int leave_namespace(struct sctx * sctx) {
    if(!sexpr_can_pop(&sctx->namespaces)) {
        return SCTX_ERROR;
    }
    sexpr_pop(&sctx->namespaces);
    return SCTX_OK;
}

static void 
shash_namespace_destructor(void * shash) {
    shash_free((struct shash_table *)shash);
    free(shash);
}

static void 
shash_namespace_visit(void * shash) {
    /* do nothing */
}

static void 
shash_namespace_mark_reachable(void * shash, unsigned char mark) {
    shash_visit((struct shash_table *) shash, NULL, mark_reachable_references_cb);
}


struct sexpression * lookup_name(struct sctx * sctx, struct sexpression * name) {
    struct sexpression * namestack;
    struct sexpression * namespace;
    struct sexpression * value;
    
    value = shash_search(&sctx->protected_namespace, name);
    
    if(value != NULL) {
        return value;
    }
    
    namestack = sctx->namespaces;
    
    while(namestack) {
        namespace = sexpr_car(namestack);
        if(namespace == NULL) continue;
        value = shash_search((struct shash_table*) sexpr_primitive_ptr(namespace), name);
        if(value != NULL) {
            return value;
        }
        namestack = sexpr_cdr(namestack);
    }
    
    return NULL;
}

struct sexpression * 
alloc_new_pair(struct sctx * sctx, struct sexpression * car, struct sexpression * cdr) {
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

struct sexpression * 
alloc_new_string(struct sctx * sctx, wchar_t * wcstr, size_t len) {
    /* future improvement: string cache */
    struct sexpression * object;
    
    object = sexpr_create_string(wcstr, len);
    
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

struct sexpression * 
alloc_new_symbol(struct sctx * sctx, wchar_t * wcstr, size_t len) {
    /* future improvement: string cache */
    struct sexpression * object;
    
    object = sexpr_create_symbol(wcstr, len);
    
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

struct sexpression *
alloc_new_primitive(struct sctx * sctx, void * ptr, struct sprimitive * handler) {
    struct sexpression * object;
    
    object = sexpr_create_primitive(ptr, handler);
    
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

struct sexpression *
alloc_new_function (struct sctx * sctx, sexpression_callable function, struct sexpression * body) {
    struct sexpression * object;
    
    object = sexpr_create_function(function, body);
    
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


struct sexpression *
alloc_new_error(struct sctx * sctx, wchar_t * wcstr, struct sexpression * call_stack) {
    struct sexpression * object;
    
    object = sexpr_create_error(wcstr, call_stack);
    
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
    
    /* visit all nodes reachable from the namespaces, including sexpr being loaded and primitives */
    shash_visit(&sctx->protected_namespace, NULL, mark_reachable_references_cb);
    sexpr_mark_reachable(sctx->in_load, 1);
    sexpr_mark_reachable(sctx->namespaces, 1);
    
    free_unvisited_references(&sctx->heap);
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(&sctx->heap);
    
}

static void free_unvisited_references(struct mem_heap * heap) {
    size_t i = 0;
    struct sexpression * sexpr;

#ifdef UNIT_TESTING
    print_heap_contents(heap, "Heap before GC");
#endif

    while(i < heap->load ) {
        sexpr = heap->data[i];
        if ( sexpr == NULL ) {
            i++;
        } else if (sexpr_is_marked ( sexpr, 1 ) ) {
            i++;
            sexpr_set_mark(sexpr, 0);
        } else {
            sexpr_free_object ( sexpr );
            heap->load--;
            heap->data[i] = heap->data[heap->load];
            heap->data[heap->load] = NULL;
        }
    }
    
#ifdef UNIT_TESTING
    print_heap_contents(heap, "Heap after GC");
#endif

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

    for(i = 0; i < heap->load; i++) {
        sexpr_free_object(data[i]);
    }
    heap->load = 0;
}

#ifdef UNIT_TESTING
static void print_heap_contents(struct mem_heap * heap, char * msg) {
    size_t i;
    struct sexpression ** data = heap->data;
    struct sexpression * obj;
    FILE * const out = stdout;
    
    fprintf(out, "HEAP CONTENTS: %s [%lu/%lu]\n", msg, (unsigned long) heap->load, (unsigned long) heap->size);
    for(i = 0; i < heap->size; i++) {
        obj = data[i];
        if(sexpr_is_string(obj)) {
            fprintf(out, "\tSTRING    object in the heap: %p => \"%.*ls\"\n", (void*)obj, (int)obj->len, obj->data.value); 
        } else if(sexpr_is_symbol(obj)) {
            fprintf(out, "\tSYMBOL    object in the heap: %p => %.*ls\n", (void*)obj, (int)obj->len, obj->data.value); 
        } else if(sexpr_is_cons(obj)) {
            fprintf(out, "\tCONS      object in the heap: %p => (%p . %p)\n", (void*)obj, (void*)obj->data.sexpr, (void*)obj->cdr.sexpr); 
        } else if(sexpr_is_function(obj)) {
            fprintf(out, "\tFUNCTION  object in the heap: %p => %p\n", (void*)obj, obj->data.ptr); 
        } else if(sexpr_is_primitive(obj)) {
            fprintf(out, "\tPRIMITIVE object in the heap: %p => %p\n", (void*)obj, obj->data.ptr);
        } else if(sexpr_is_nil(obj)) {
            fprintf(out, "\tNIL       object in the heap\n");
        } else {
            fprintf(out, "\tFUNNY object in the heap: %p => type[%d]\n", (void*)obj, sexpr_type(obj)); 
        }        
    }
    fflush(out);
}
#endif


int
move_to_heap(struct sctx * sctx, struct sexpression * sexpr) {
    if(sctx == NULL) {
        return SCTX_ERROR;
    }
    
    struct scallback callback = {
        .enter = NULL,
        .visit = move_to_heap_visitor,
        .leave = NULL,
        .context = sctx,
        .state = SCTX_OK
    };
    
    
    sctx->in_load = sexpr;
    
    // do stuff
    svisitor(sexpr, &callback);
    
    sctx->in_load = NULL;
    
    return callback.state;
}

static void 
move_to_heap_visitor(struct sexpression * sexpr, struct scallback * callback) {
    /* do nothing if the previous state is not 0 (OK) */
    if(callback->state != SCTX_OK) return;
    
    callback->state = record_new_object((struct sctx *) callback->context, sexpr);
    
}
