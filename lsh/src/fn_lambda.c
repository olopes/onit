#include <stdlib.h>
#include "core_functions.h"

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);

static enum sexpression_result 
_fn_lambda_compiler(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    
    alloc_new_function(sctx, _fn_lambda_evaluator, closure);
    
    return FN_OK;
}

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    
    
    return FN_OK;
}



sexpression_callable fn_lambda = _fn_lambda_compiler;
