#define _XOPEN_SOURCE 700
#include <features.h>
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
#include "core_functions.h"

#ifdef putwchar
#   undef putwchar
#endif

static struct sctx_config coalesce_config(struct sctx_config * configuration);
static int load_array_to_sexpr(struct sctx * sctx, struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static void discover_functions(struct sctx * sctx, struct core_function_definition const * const start, struct core_function_definition const * const end);
static wchar_t * convert_to_wcstr(const char * src);
static void free_heap_contents (struct mem_heap * heap);
static int record_new_object(struct sctx * sctx, struct sexpression * obj);
static void recycle(struct sctx * sctx);
static void free_unvisited_references(struct mem_heap * heap);
static void grow_heap_if_necessary(struct sctx * sctx);
static inline int heap_should_grow(struct sctx * sctx);
static void mark_reachable_references_cb (void * sctx_ptr, struct sexpression * key, void * reference);
static int 
create_reference(struct shash_namespace * namespace, struct sexpression * name, struct mem_reference * reference);
static void 
move_to_heap_visitor(struct sexpression * sexpr, struct scallback * callback);

#ifdef UNIT_TESTING
static void heap_sanity_check(struct mem_heap * heap, struct sexpression * obj);
#  ifdef DUMP_HEAP_CONTENTS
static void print_heap_contents(struct mem_heap * heap, char * msg);
#  endif
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
create_new_sctx(struct sctx_config * configuration) {
    struct sctx * sctx;
    struct sexpression ** heap;
    struct sctx_config config = coalesce_config(configuration);
    
    sctx = (struct sctx *) malloc(sizeof(struct sctx));
    if(sctx == NULL) {
        return NULL;
    }
    
    heap = (struct sexpression **) malloc(sizeof(struct sexpression *) * config.heap_min_size);
    if(heap == NULL) {
        free(sctx);
        return NULL;
    }
    
    memset(heap, 0, sizeof(struct sexpression *) * config.heap_min_size);
    
    *sctx = (struct sctx) {
        .configuration = config,
        .protected_namespace = {
            .table = {
                .size=0,
                .load=0,
                .table=NULL,
            },
            .temp_length=0,
            .temp_entries=NULL,
        },
        .global_namespace = NULL,
        .namespaces = NULL,
        .in_load = NULL,
        .heap = (struct mem_heap) {
            .visit = 0,
            .size = config.heap_min_size,
            .load = 0,
            .data = heap,
        },
        .print_char = putwchar
    };
    
    
    /* setup locale? or assume already configured? */
    
    /* create and keep reference for global namespace */
    enter_namespace(sctx);
    sctx->global_namespace = (struct shash_namespace *) sexpr_primitive_ptr(sexpr_peek(&sctx->namespaces));
    
    load_primitives(sctx);
    
    /* create two references */
    /* register args and env. forget about primitives*/

    /* TODO extract function  ? */
    struct mem_reference reference;
    create_global_reference(sctx, ARGUMENTS_SYMBOL_NAME, ARGUMENTS_SYMBOL_NAME_LENGTH, &reference);
    load_array_to_sexpr(sctx, (struct sexpression **) reference.value, config.argv);
    create_global_reference(sctx, ENVIRONMENT_SYMBOL_NAME, ENVIRONMENT_SYMBOL_NAME_LENGTH, &reference);
    load_array_to_sexpr(sctx, (struct sexpression **) reference.value, config.envp);
    
    return sctx;
}

static struct sctx_config config_prototype = {
    .argv = NULL,
    .envp = NULL,
    .heap_min_size = 128,
    .heap_max_size = 131072,
    .register_static_functions = 1,
    .register_dynamic_functions = 1
};    


static struct sctx_config coalesce_config(struct sctx_config * configuration) {
    struct sctx_config normalized;
    
    if (configuration == NULL) {
        return config_prototype;
    }
    
    normalized = *configuration;
    
    if (normalized.heap_min_size == 0) {
        normalized.heap_min_size = config_prototype.heap_min_size;
    }
    
    if (normalized.heap_max_size == 0) {
        normalized.heap_max_size = config_prototype.heap_max_size;
    }
    
    return normalized;
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



/* TODO move this to another file? how to preserve the static nature? */

static void register_primitive_symbols(struct sctx * sctx);
static void discover_static_functions(struct sctx * sctx);
static void discover_dynamic_loaded_functions(struct sctx * sctx);


static void load_primitives(struct sctx * sctx) {
    register_primitive_symbols(sctx);

    if (sctx->configuration.register_static_functions) {
        discover_static_functions(sctx);
    }
    
    if (sctx->configuration.register_dynamic_functions) {
        discover_dynamic_loaded_functions(sctx);
    }
}

static void register_primitive_symbols(struct sctx * sctx) {
    struct mem_reference reference;
    create_protected_reference (sctx, L"#t", 2, &reference);
    *reference.value = *reference.key;
    create_protected_reference (sctx, L"#f", 2, &reference);
    *reference.value = NULL;
    
    create_protected_reference (sctx, L"#nl", 3, &reference);
#ifdef _WIN32
    *reference.value = alloc_new_string(sctx, L"\r\n", 2);
#else
    *reference.value = alloc_new_string(sctx, L"\n", 1);
#endif
}

// load from linker scripts

/* injected automatically by the linker script */
extern struct core_function_definition __start_lsh_fn;
extern struct core_function_definition __stop_lsh_fn;

// TODO error checks
static void discover_static_functions(struct sctx * sctx) {
    discover_functions(sctx, &__start_lsh_fn, &__stop_lsh_fn);
}


static void discover_functions(struct sctx * sctx, struct core_function_definition const * const start, struct core_function_definition const * const end) {
    struct core_function_definition const * ptr;
    struct mem_reference reference;

    for(ptr = start; ptr < end; ptr++) {
        create_protected_reference (sctx, ptr->name, wcslen(ptr->name), &reference);
        *reference.value = alloc_new_function(sctx, ptr->fn_ptr, NULL);
    }
}

// load from dl_open
// TODO move somewhere else
#include <dlfcn.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>

struct dynamic_library {
    void * handle;
    char * path;
};

static void load_dynamic_libraries_in_dir (struct sctx * sctx, char * basepath);
static void discover_functions_in_library(struct sctx * sctx, struct dynamic_library * lib);

static void discover_dynamic_loaded_functions(struct sctx * sctx) {

    /* TODO if stuff according to OS */
    load_dynamic_libraries_in_dir (sctx, "./lib");
    load_dynamic_libraries_in_dir (sctx, "~/.lsh/lib");
    /* ignore (basename)/bin if . is same as (basename) */
    
    if (sctx->configuration.argv != NULL && sctx->configuration.argv[0] != NULL) {
        char * cwd = realpath(".", NULL);
        size_t cwd_size = strlen(cwd);
    
        char * bin_name = realpath(sctx->configuration.argv[0], NULL);
        char * bin_basedir_end = strrchr(bin_name, '/');
        size_t bin_size = bin_basedir_end - bin_name;
    
        
        if (bin_size != cwd_size || strncmp(cwd, bin_name, cwd_size) != 0) {
            char * bin_lib = (char*) malloc(sizeof(char) * (bin_size + 5));
            snprintf(bin_lib, (bin_size + 5), "%.*s/bin", (int)bin_size, bin_name);
            load_dynamic_libraries_in_dir (sctx, bin_lib);
            free(bin_lib);
        }
        /* find_dynamic_libraries_in_dir(sctx, "$PREFIX/lib/lsh"); */ 
        
        free(bin_name);
        free(cwd);
    }
}
    

static void load_dynamic_libraries_in_dir (struct sctx * sctx, char * basepath) {
    DIR *dirp;
    struct dirent *dp;
    size_t base_len = strlen(basepath);
    
    if ((dirp = opendir(basepath)) == NULL) return; /* couldn't open dir, quit. */
    
    do {
        errno = 0;
        if ((dp = readdir(dirp)) == NULL) break;

        size_t name_len = strlen(dp->d_name);
        if (name_len < 4) continue;
        
        /* doesn't end with .so */
        if (strcmp(dp->d_name+(name_len-4), ".so") != 0) continue;

        char * file_name = (char *) malloc(sizeof(char)*(base_len + name_len + 2));
        
        snprintf(file_name, (base_len + name_len + 2), "%s/%s", basepath, dp->d_name);
        
        struct dynamic_library * lib_reference = (struct dynamic_library*) malloc(sizeof(struct dynamic_library));
        *lib_reference = (struct dynamic_library){
            .handle = dlopen(file_name, RTLD_LAZY | RTLD_LOCAL),
            .path = file_name
        };
        
        discover_functions_in_library(sctx, lib_reference);
        
        sexpr_push(&sctx->libraries, (struct sexpression *) lib_reference);
        
    } while (dp != NULL);
    
    (void) closedir(dirp);
    return;
    
}

static void discover_functions_in_library(struct sctx * sctx, struct dynamic_library * lib) {
    if(lib->handle == NULL) return;
    struct core_function_definition * start_lsh_fn = dlsym(lib->handle, "__start_lsh_fn");
    struct core_function_definition * stop_lsh_fn = dlsym(lib->handle, "__stop_lsh_fn");

    if(start_lsh_fn == NULL || stop_lsh_fn == NULL) return;
    
    discover_functions(sctx, start_lsh_fn, stop_lsh_fn);

}


// end dlopen stuff

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
    
    return create_reference((struct shash_namespace *) sexpr_primitive_ptr(namespace), name, reference);
}

static int 
create_reference(struct shash_namespace * namespace, struct sexpression * name, struct mem_reference * reference) {
    struct shash_entry * entry;
    struct shash_table * table = &namespace->table;
    
    if(!shash_has_key(table, name)) {
        shash_insert(table, name, NULL);
    }
    
    entry = shash_get_entry(table, name);
    
    *reference = (struct mem_reference) {
        .key = &entry->key,
        .value = (struct sexpression **) &entry->value,
    };
    
    return SCTX_OK;
}


#define TEMP_SEGMENT_SIZE 32
int
create_temporary_reference (struct sctx * sctx, struct mem_reference * reference) {
    struct sexpression * namespace_sexpr;
    struct shash_namespace * namespace;
    
    if (sctx == NULL || reference == NULL) {
        return SCTX_ERROR;
    }
    
    namespace_sexpr = sexpr_peek(&sctx->namespaces);
    if(namespace_sexpr == NULL) {
        return SCTX_ERROR;
    }
    
    namespace = (struct shash_namespace *) sexpr_primitive_ptr(namespace_sexpr);
    if(namespace == NULL) {
        return SCTX_ERROR;
    }
    
    if ((namespace->temp_length & TEMP_SEGMENT_SIZE) == 0) {
        size_t new_size = namespace->temp_length + TEMP_SEGMENT_SIZE;
        struct sexpression ** entries  = realloc(namespace->temp_entries, sizeof(struct sexpression *) * new_size);
        if (entries == NULL) {
            return SCTX_ERROR;
        }
        memset(entries + namespace->temp_length, 0, TEMP_SEGMENT_SIZE);
        namespace->temp_entries = entries;
    }
    
    reference->key = NULL;
    reference->value = namespace->temp_entries + namespace->temp_length;
    namespace->temp_length++;
    
    return SCTX_OK;
}


void 
release_sctx(struct sctx * sctx) {
    
    if(sctx == NULL) {
        return;
    }
    
    
    shash_free(&sctx->protected_namespace.table);

    leave_namespace(sctx);
    /* sctx_gc(sctx); */
    
    free_heap_contents(&sctx->heap);
    
    free(sctx->heap.data);
    free(sctx);
}

int enter_namespace(struct sctx * sctx) {
    struct shash_namespace * new_namespace;
    struct sexpression * primitive;
    new_namespace = (struct shash_namespace *) malloc(sizeof(struct shash_namespace));
    if(new_namespace == NULL) {
        return SCTX_ERROR;
    }
    memset(new_namespace, 0, sizeof(struct shash_namespace));
    
    primitive = alloc_new_primitive(sctx, new_namespace, &shash_namespace_handler );
    if(primitive == NULL) {
        shash_free(&new_namespace->table);
        free(new_namespace);
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
    struct shash_namespace * namespace = (struct shash_namespace *) shash;
    shash_free(&namespace->table);
    
    if(namespace->temp_entries) {
        free(namespace->temp_entries);
    }
    free(namespace);
}

static void 
shash_namespace_visit(void * shash) {
    /* do nothing */
}

static void 
shash_namespace_mark_reachable(void * shash, unsigned char mark) {
    size_t i;
    struct shash_namespace * namespace = (struct shash_namespace *) shash;
    
    shash_visit(&namespace->table, NULL, mark_reachable_references_cb);
    
    if(namespace->temp_entries != NULL && namespace->temp_length > 0) {
        for (i = 0; i < namespace->temp_length; i++) {
            sexpr_mark_reachable(namespace->temp_entries[i], mark);
        }   
    }
}


int lookup_name(struct sctx * sctx, struct sexpression * name, struct sexpression ** value) {
    struct sexpression * namestack;
    struct sexpression * namespace;
    struct shash_entry * entry;
    
    entry = shash_get_entry(&sctx->protected_namespace.table, name);
    
    if(entry != NULL) {
        *value = entry->value;
        return 1;
    }
    
    namestack = sctx->namespaces;
    
    while(namestack) {
        namespace = sexpr_car(namestack);
        if(namespace == NULL) continue;
        entry = shash_get_entry((struct shash_table*) sexpr_primitive_ptr(namespace), name);
        if(entry != NULL) {
            *value = entry->value;
            return 1;
        }
        namestack = sexpr_cdr(namestack);
    }
    
    *value = NULL;
    return 0;
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
alloc_new_function (struct sctx * sctx, sexpression_callable function, struct sexpression * closure) {
    struct sexpression * object;
    
    object = sexpr_create_function(function, closure);
    
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
    shash_visit(&sctx->protected_namespace.table, NULL, mark_reachable_references_cb);
    sexpr_mark_reachable(sctx->in_load, 1);
    sexpr_mark_reachable(sctx->namespaces, 1);
    
    free_unvisited_references(&sctx->heap);
    
    /* grow heap  if load >= 50% of the heap size */
    grow_heap_if_necessary(sctx);
    
}

static void free_unvisited_references(struct mem_heap * heap) {
    size_t i = 0;
    struct sexpression * sexpr;

#if defined(UNIT_TESTING) && defined(DUMP_HEAP_CONTENTS)
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
    
#if defined(UNIT_TESTING) && defined(DUMP_HEAP_CONTENTS)
    print_heap_contents(heap, "Heap after GC");
#endif

}


static void grow_heap_if_necessary(struct sctx * sctx) {
    struct sexpression ** data;
    size_t new_size;
    
    if(heap_should_grow(sctx)) {
        new_size = sctx->heap.size << 1;
        data = (struct sexpression **) realloc(sctx->heap.data, sizeof(struct sexpression *)*new_size);
        if(data != NULL) {
            memset(data, 0, sizeof(struct sexpression *) * (new_size - sctx->heap.load ));
            sctx->heap.size = new_size;
            sctx->heap.data = data;
        }
    }
}

static inline int heap_should_grow(struct sctx * sctx) {
    return (sctx->heap.size < sctx->configuration.heap_min_size && sctx->heap.load*2 > sctx->heap.size );
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

#if defined(UNIT_TESTING) && defined(DUMP_HEAP_CONTENTS)
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
    
    if(sexpr != NULL)
        callback->state = record_new_object((struct sctx *) callback->context, sexpr);
    
}
