#include <stdlib.h>
#include "sexpr.h"
#include "sobj.h"
#include "sexpr_stack.h"

/**
 * Push a S-Expression object into the top of the stack
 */
void WEAK_FOR_UNIT_TEST
sexpr_push(struct sexpr ** sexpr, struct sobj * obj) {
    struct sobj * cdr;
    struct sexpr * node;
    
    cdr = sobj_from_sexpr(*sexpr);
    
    node = sexpr_cons(obj, cdr);
    
    /* TODO handle NULL return */
    
    *sexpr = node;
}

/**
 * Fetch the top most element of the stack
 */
struct sobj * WEAK_FOR_UNIT_TEST
sexpr_peek(struct sexpr ** sexpr) {
    return sexpr_car(*sexpr);
}

/**
 * Remove and return the top-most element of the stack
 */
struct sobj * WEAK_FOR_UNIT_TEST
sexpr_pop(struct sexpr ** sexpr) {
    struct sexpr * node;
    struct sobj * value;
    struct sobj * cdr;
    value = sexpr_car(*sexpr);
    cdr = sexpr_cdr(*sexpr);
    node = *sexpr;
    *sexpr = sobj_to_sexpr(cdr);
    sobj_free(cdr);
    sexpr_free(node);
    return value;
}

/**
 * Returns TRUE if the stack can pop the top-most element, ie, is not empty.
 */
int WEAK_FOR_UNIT_TEST
sexpr_can_pop(struct sexpr * sexpr) {
    return sexpr && sobj_is_cons(sexpr_cdr(sexpr));
}

