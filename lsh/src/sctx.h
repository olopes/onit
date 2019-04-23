#ifndef __SCTX_H__
#define __SCTX_H__

#include <stddef.h>
#include "sexpr.h"
#include "ostr.h"
#include "shash.h"

struct primitive;
struct sctx;
typedef struct sexpression * (*primitive_fn)(void * sctx, struct sexpression  *);
typedef void (*destructor_fn)(struct sctx * sctx, struct primitive *);

#define SCTX_OK 0
#define SCTX_ERROR 1

#define PRIMITIVE_FUNCTION 0
#define PRIMITIVE_SEXPRESSION 1
#define PRIMITIVE_STRING 2

struct primitive {
    unsigned int type;
    union {
        primitive_fn function;
        struct sexpression * sexpression;
        struct ostr * string;
    } value;
    destructor_fn destructor;
};


struct mem_heap {
    int visit;
    size_t size;
    size_t load;
    struct sexpression ** data;
};

struct sctx {
    struct shash_table primitives;
    struct shash_table * global;
    struct sexpression * namespaces;
    struct sexpression * in_load;
    struct mem_heap heap;
    void (*namespace_destructor)(struct sctx * ctx);
};

struct mem_reference {
    struct sexpression ** key;
    void ** value;
};

#define HEAP_MIN_SIZE 32
#define HEAP_MAX_SIZE 131072


extern struct sctx * 
create_new_sctx(char **argv, char **envp);

extern void 
release_sctx(struct sctx * sctx);

extern void 
sctx_gc(struct sctx * sctx);

extern int
create_global_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern int
create_stack_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern int
create_primitive_reference(struct sctx * sctx, wchar_t * wcstr, size_t len, struct mem_reference * reference);

extern struct sexpression * 
alloc_new_pair(struct sctx * sctx, struct sexpression * car, struct sexpression * cdr);

extern struct sexpression * 
alloc_new_value(struct sctx * sctx, wchar_t * wcstr, size_t len);

extern int 
enter_namespace(struct sctx * sctx);

extern int 
leave_namespace(struct sctx * sctx);

extern struct sexpression * 
lookup_name(struct sctx * sctx, struct sexpression * name);

extern int
move_to_heap(struct sctx * sctx, struct sexpression * sexpr);

#endif
