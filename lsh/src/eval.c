#include "eval.h"
#include "sexpr.h"
#include "sctx.h"

static struct sexpression * 
apply_sexpr(struct sctx * sctx, struct sexpression * fn,  struct sexpression * args);

/**
 * Eval a S-Expression using the given context
 */
struct sexpression * 
eval_sexpr(struct sctx * sctx, struct sexpression * expression) {
    if(sctx == NULL || expression == NULL) {
        return NULL;
    }
    
    if(sexpr_is_cons(expression)) {
        struct sexpression * fn = eval_sexpr(sctx, sexpr_car(expression));
        
        if(sexpr_is_function(fn))  {
            return apply_sexpr(sctx, fn, sexpr_cdr(expression));
        }
        
        return alloc_new_error(sctx, L"yikes!", fn);
    }
    
    if(sexpr_is_symbol(expression)) {
        return lookup_name(sctx, expression);
    }
    return expression;
    
}

static struct sexpression * 
apply_sexpr(struct sctx * sctx, struct sexpression * fn,  struct sexpression * args) {
    struct sexpression * result;
    struct sexpression * body;
    sexpression_callable function;
    
    enter_namespace(sctx);
    body = sexpr_function_body(fn);
    function = sexpr_function(fn);

    result = function(sctx, body, args);
    
    leave_namespace(sctx);
    return result;
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



