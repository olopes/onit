#include <stdlib.h>
#include "sexpr.h"
#include "sexpr_stack.h"

/**
 * Push a S-Expression object into the top of the stack
 */
void 
sexpr_push(struct sexpression ** sexpr, void * obj) {
    struct sexpression * node;
    
    node = sexpr_cons((struct sexpression *) obj, *sexpr);
    
    /* TODO handle NULL return */
    
    *sexpr = node;
}

/**
 * Fetch the top most element of the stack
 */
void *
sexpr_peek(struct sexpression ** sexpr) {
    return sexpr_car(*sexpr);
}

/**
 * Remove and return the top-most element of the stack
 */
void *
sexpr_pop(struct sexpression ** sexpr) {
    void * car;
    struct sexpression * cdr;

    car = sexpr_car(*sexpr);
    cdr = sexpr_cdr(*sexpr);
    sexpr_free_object(*sexpr);

    *sexpr = cdr;
    
    return car;
}

/**
 * Returns TRUE if the stack can pop the top-most element, ie, is not empty.
 */
int
sexpr_can_pop(struct sexpression ** sexpr) {
    return sexpr_is_cons(*sexpr);
}

