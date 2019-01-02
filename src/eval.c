
#include "eval.h";
#include "sexpr.h";
#include "ostr.h";

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

static struct sexpr * 
sexpr_eval_expand(struct sexpr * list, struct sexpr * accum) {
    struct sexpr_obj * value;
    if(list->car->type == sexpr) {
        value = sexpr_eval(sexpr_obj_to_sexpr(list->car));
    } else {
        value = list->car;
    }
    
    return sexpr_cons(value, list->cdr);
}

static struct sexpr_obj * 
sexpr_eval_reduce(struct sexpr * list) {
    /* TODO oh dear... please help me Mr. Sussman!! */
    
    return list->car; /* dummy implementation for now */
}



/**
 * Eval a S-Expression
 */
struct sexpr_obj * 
eval_sexpr(struct sexpr_obj * sexpr) {
    struct sexpr * accum;
    struct sexpr_obj * result;
    
    if(obj == NULL || obj->type != lst) {
        return obj;
    }
    
    accum = sexpr_cons(NULL, NULL);
    
    sexpr_eval_expand(sexpr, accum);
    
    
    
    return result; /* do nothing for now */
}



