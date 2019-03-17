#ifndef __SCTX_H__
#define __SCTX_H__

#include <stddef.h>
#include "sexpr.h"
#include "ostr.h"

struct primitive;
typedef struct sexpression * (*primitive_fn)(void * sctx, struct sexpression  *);
typedef void (*destructor_fn)(void * sctx, struct primitive *);

#define PRIMITIVE_FUNCTION 0
#define PRIMITIVE_SEXPRESSION 1
#define PRIMITIVE_VALUE 2
#define PRIMITIVE_STRING 3

struct primitive {
    unsigned int type;
    union {
        primitive_fn function;
        struct sexpression * sexpression;
        struct svalue * value;
        struct ostr * string;
    } value;
    destructor_fn destructor;
};



extern void * 
init_environment(char **argv, char **envp);

extern void 
release_environment(void * sctx);

extern void 
sctx_gc(void * sctx);

extern int 
register_primitive(void * sctx, struct svalue * name, struct primitive * primitive);

extern struct sexpression * 
alloc_new_pair(void * sctx, struct sexpression * car, struct sexpression * cdr);

extern struct sexpression * 
alloc_new_value(void * sctx, wchar_t * wcstr, size_t len);

extern int 
enter_namespace(void * sctx);

extern int 
leave_namespace(void * sctx);

extern struct sexpression * 
lookup_name(void * sctx, struct svalue * name);

extern int 
register_value(void * sctx, struct svalue * name, struct sexpression * value);

#endif
