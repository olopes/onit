#ifndef __CORE_FUNCTIONS_H__
#define __CORE_FUNCTIONS_H__

#include "sexpr.h"
#include "sctx.h"

enum sexpression_result 
fn_procedure_step(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression);

struct core_function {
    char * name;
    sexpression_callable fn_ptr;
} __attribute__((aligned(64)));


#define CoreFunction(fn_name) \
static enum sexpression_result \
_fn_ ## fn_name(struct sctx *, struct sexpression **, struct sexpression *, struct sexpression *); \
sexpression_callable fn_ ## fn_name = _fn_ ## fn_name; \
struct core_function  __attribute__ ((section ("lsh_fn"))) _def ## fn_name = { \
    .name = #fn_name , \
    .fn_ptr = _fn_ ## fn_name \
}; \
static enum sexpression_result \
_fn_ ## fn_name(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * arguments)


extern sexpression_callable fn_define;

extern sexpression_callable fn_lambda;

extern sexpression_callable fn_procedure;

extern sexpression_callable fn_echo;


#endif
