#include "eval.h"
#include "sexpr.h"
#include "ostr.h"

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



/**
 * Eval a S-Expression
 */
WEAK_FOR_UNIT_TEST struct sexpression * 
eval_sexpr(struct sexpression * sexpr) {
    struct sexpression * accum;
    struct sexpression * result;
    
    if(sexpr == NULL || !sexpr_is_cons(sexpr)) {
        return sexpr;
    }
    
    accum = sexpr_cons(NULL, NULL);
    
    result = sexpr_eval_expand(sexpr, accum);
    
    
    
    return result; /* do nothing for now */
}



