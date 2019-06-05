#include <stdlib.h>
#include "core_functions.h"

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);

static enum sexpression_result 
_fn_lambda_compiler(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    
    if(sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if(result == NULL) {
        return FN_NULL_RESULT;
    }
    
    
    *result = alloc_new_function(sctx, _fn_lambda_evaluator, closure);
    
    return FN_OK;
}

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    
    if(sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if(result == NULL) {
        return FN_NULL_RESULT;
    }
    
    
    
    return FN_OK;
}



sexpression_callable fn_lambda = _fn_lambda_compiler;
