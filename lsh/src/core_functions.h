#ifndef __CORE_FUNCTIONS_H__
#define __CORE_FUNCTIONS_H__

#include "sexpr.h"
#include "sctx.h"

enum sexpression_result 
fn_procedure_step(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression);

extern sexpression_callable fn_define;

extern sexpression_callable fn_lambda;

extern sexpression_callable fn_procedure;

extern sexpression_callable fn_echo;

#endif
