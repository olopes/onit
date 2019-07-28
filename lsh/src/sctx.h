#ifndef __SCTX_H__
#define __SCTX_H__

#include <wchar.h>
#include <stdint.h>
#include "sexpr.h"
#include "ostr.h"
#include "shash.h"

#define SCTX_OK 0
#define SCTX_ERROR 1

struct mem_heap {
    int visit;
    size_t size;
    size_t load;
    struct sexpression ** data;
};

struct shash_namespace {
    struct shash_table table;
    size_t temp_length;
    struct sexpression ** temp_entries;
};

struct sctx_config {
    char **argv;
    char **envp;
    /* not implemented yet... */
    size_t heap_min_size;
    size_t heap_max_size;
    unsigned int register_static_functions : 1;
    unsigned int register_dynamic_functions : 1;
};    

struct sctx {
    struct sctx_config configuration;
    struct shash_namespace protected_namespace;
    struct shash_namespace * global_namespace;
    struct sexpression * namespaces;
    struct sexpression * in_load;
    struct sexpression * libraries;
    struct mem_heap heap;
    wint_t (*print_char)(wchar_t chr);
};

struct mem_reference {
    struct sexpression ** key;
    struct sexpression ** value;
};


extern struct sctx * 
create_new_sctx(struct sctx_config * configuration);

extern void 
release_sctx(struct sctx * sctx);

extern void 
sctx_gc(struct sctx * sctx);

extern int
create_global_reference (struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern int
create_stack_reference (struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern int
create_protected_reference (struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern int
create_temporary_reference (struct sctx * sctx, struct mem_reference * reference);

extern struct sexpression * 
alloc_new_pair(struct sctx * sctx, struct sexpression * car, struct sexpression * cdr);

extern struct sexpression * 
alloc_new_string(struct sctx * sctx, wchar_t * wcstr, size_t len);

extern struct sexpression * 
alloc_new_symbol(struct sctx * sctx, wchar_t * wcstr, size_t len);

extern struct sexpression *
alloc_new_primitive(struct sctx * sctx, void * ptr, struct sprimitive * handler);

extern struct sexpression *
alloc_new_function (struct sctx * sctx, sexpression_callable function, struct sexpression * body);

extern struct sexpression *
alloc_new_error(struct sctx * sctx, wchar_t * wcstr, struct sexpression * call_stack);

extern int 
enter_namespace(struct sctx * sctx);

extern int 
leave_namespace(struct sctx * sctx);

extern int 
lookup_name(struct sctx * sctx, struct sexpression * name, struct sexpression ** value);

extern int
move_to_heap(struct sctx * sctx, struct sexpression * sexpr);

#endif
