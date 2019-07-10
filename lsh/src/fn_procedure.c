#include <stdlib.h>
#include "core_functions.h"

static enum sexpression_result 
apply_sexpr(struct sctx * sctx, struct sexpression ** result, struct sexpression * fn,  struct sexpression * args);


CoreFunction(procedure) {
    (void) closure;
    enum sexpression_result return_value;
    struct sexpression * iter;
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if (arguments == NULL) {
        /* send pretty error message? */
        return FN_ERROR;
    }
    
    iter = arguments;
    
    while(iter) {
        struct sexpression * expression = sexpr_car(iter);
        
        return_value = fn_procedure_step(sctx, result, expression);
        
        if(return_value != FN_OK) {
            /* something happened. break the loop! */
            break;
        }
        iter = sexpr_cdr(iter);
    }
    
    return return_value;
}

enum sexpression_result 
fn_procedure_step(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression) {
    enum sexpression_result return_value = FN_OK;
    
    if(sexpr_is_cons(expression)) {
        struct sexpression * fn;
        if(fn_procedure_step(sctx, &fn, sexpr_car(expression)) != FN_OK) {
            *result = fn;
            return_value = FN_ERROR;
        } else if(sexpr_is_function(fn))  {
            return_value = apply_sexpr(sctx, result, fn, sexpr_cdr(expression));
        } else {
            *result = alloc_new_error(sctx, L"yikes!", fn);
            return_value = FN_ERROR;
        }
        
    } else if(sexpr_is_symbol(expression)) {
        *result = lookup_name(sctx, expression);
    } else {
        *result = expression;
    }
        
    return return_value;
}

static enum sexpression_result 
apply_sexpr(struct sctx * sctx, struct sexpression ** result, struct sexpression * fn,  struct sexpression * args) {
    struct sexpression * closure;
    sexpression_callable function;
    enum sexpression_result return_value;
    
    enter_namespace(sctx);
    closure = sexpr_function_closure(fn);
    function = sexpr_function(fn);

    return_value = function(sctx, result, closure, args);
    
    leave_namespace(sctx);
    return return_value;
}






/* ************************************************************* */


/*
O objectivo é passar disto:
((a b c) (c d) e f)
para isto:
((a b c) (c d) e f) =>
((ab c) (c d) e f) =>
((abc) (c d) e f) =>
(abc (c d) e f) =>
(abc (cd) e f) =>
(abc cd e f)
*/

WEAK_FOR_UNIT_TEST struct sexpression * 
sexpr_eval_expand(struct sexpression * list, struct sexpression * accum) {
    struct sexpression * value;
    if(sexpr_is_cons(list)) {
        value = NULL;/* sexpr_eval(sobj_to_sexpr(list->car)); */
    } else {
        value = sexpr_car(list);
    }
    
    return sexpr_cons(value, sexpr_cdr(list));
}

WEAK_FOR_UNIT_TEST struct sexpression * 
sexpr_eval_reduce(struct sexpression * list) {
    /* TODO oh dear... please help me Mr. Sussman!! */
    
    return sexpr_car(list); /* dummy implementation for now */
}




