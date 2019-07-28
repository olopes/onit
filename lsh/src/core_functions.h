#ifndef __CORE_FUNCTIONS_H__
#define __CORE_FUNCTIONS_H__

#include "sexpr.h"
#include "sctx.h"

enum sexpression_result 
fn_procedure_step(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression);

struct core_function_definition {
    wchar_t * name;
    sexpression_callable fn_ptr;
} __attribute__((aligned(64)));


/**
 * Signature of the function called after dl_open
 */
typedef int (*dl_core_function)(struct core_function_definition *);


#define CoreFunctionN(fptr, fname) \
static enum sexpression_result \
_fn_ ## fptr(struct sctx *, struct sexpression **, struct sexpression *, struct sexpression *); \
sexpression_callable fn_ ## fptr = _fn_ ## fptr; \
struct core_function_definition  __attribute__ ((section ("lsh_fn"))) _def ## fptr = { \
    .name = fname , \
    .fn_ptr = _fn_ ## fptr \
}; \
static enum sexpression_result \
_fn_ ## fptr(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * arguments)


#define CoreFunction(fptr) CoreFunctionN(fptr, L ## #fptr)


extern sexpression_callable fn_define;

extern sexpression_callable fn_lambda;

extern sexpression_callable fn_procedure;

extern sexpression_callable fn_echo;

extern sexpression_callable fn_cond;


#endif
