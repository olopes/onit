#include <stdlib.h>
#include "sexpr.h"
#include "sexpr_stack.h"

/**
 * Push a S-Expression object into the top of the stack
 */
void WEAK_FOR_UNIT_TEST
sexpr_push(struct sexpr ** sexpr, struct sobj * obj) {
    struct sexpression * node;
    
    node = sexpr_cons(obj, *sexpr);
    
    /* TODO handle NULL return */
    
    *sexpr = node;
}

/**
 * Fetch the top most element of the stack
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_peek(struct sexpression ** sexpr) {
    return sexpr_car(*sexpr);
}

/**
 * Remove and return the top-most element of the stack
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_pop(struct sexpression ** sexpr) {
    struct sexpression * car;
    struct sexpression * cdr;

    car = sexpr_car(*sexpr);
    cdr = sexpr_cdr(*sexpr);
    sexpr_free_pair(*sexpr); /* TODO define a free_pair() only */

    *sexpr = cdr;
    
    return value;
}

/**
 * Returns TRUE if the stack can pop the top-most element, ie, is not empty.
 */
int WEAK_FOR_UNIT_TEST
sexpr_can_pop(struct sexpression * sexpr) {
    return sexpr && sexpr_is_cons(sexpr_cdr(sexpr));
}

