#include <stdlib.h>
#include "core_functions.h"

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);
static int
closure_arguments_are_valid(struct sexpression * closure_arguments);

static enum sexpression_result 
_fn_lambda_compiler(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    enum sexpression_result return_value;
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if (!sexpr_is_cons(parameters)) {
        /* store descriptive error in result */
        return FN_ERROR;
    }
    
    if (closure_arguments_are_valid(sexpr_car(parameters))) {
        *result = alloc_new_function(sctx, _fn_lambda_evaluator, parameters);
        return_value = FN_OK;
    } else {
        /* store descriptive error in result */
        return_value = FN_ERROR;
    }
    
    return return_value;
}

static int
closure_arguments_are_valid(struct sexpression * closure_arguments) {
    struct sexpression * iter;
    struct sexpression * arg;
    struct sexpression * tail;
    
    switch(sexpr_type(closure_arguments)) {
    case ST_NIL:
    case ST_SYMBOL:
        return 1;
    case ST_CONS:
        break;
    default:
        /* store descriptive error in result */
        return 0;
    }
    
    iter = closure_arguments;
    while (1) {
        
        arg = sexpr_car(iter);
        if (!sexpr_is_nil(arg) && !sexpr_is_symbol(arg)) {
            /* store descriptive error in result */
            return 0;
        }
        
        /* last element might be a simple pair and not a list */
        
        if (sexpr_is_cons(iter)) {
            tail = sexpr_cdr(iter);
            if (sexpr_is_nil(tail)) {
                /* end of the list */
                break;
            } else if (sexpr_is_symbol(tail)) {
                /* this is a simple pair and CDR is symbol */
                break;
            } else if (sexpr_is_cons(tail)) {
                /* this is a list - evaluate the tail*/
                iter = tail;
            } else {
                /* store descriptive error in result */
                return 0;
            }
        } else {
            /* unexpected type - send pretty error */
            return 0;
        }
    }
    
    return 1;
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
