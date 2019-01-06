#include <errno.h>
#include <stdlib.h>
#include "stype.h"
#include "sexpr.h"
#include "sobj.h"
#include "sexpr_stack.h"

/**
 * Constructs a pair (A . B)
 */
struct sexpr * WEAK_FOR_UNIT_TEST
sexpr_cons(struct sobj * car, struct sobj * cdr) {
    struct sexpr * sexpr;
    
    sexpr = (struct sexpr *) malloc(sizeof(struct sexpr));
    if(sexpr == NULL) {
        return NULL;
    }
    
    sexpr->car = car;
    sexpr->cdr = cdr;
    
    return sexpr;
}

void WEAK_FOR_UNIT_TEST
sexpr_free(struct sexpr * sexpr) {
    free(sexpr);
}

/**
 * Get the CAR part
 */
struct sobj * WEAK_FOR_UNIT_TEST
sexpr_car(struct sexpr *sexpr) {
    return sexpr->car;
}

/**
 * Get the CDR part
 */
struct sobj * WEAK_FOR_UNIT_TEST
sexpr_cdr(struct sexpr *sexpr) {
    return sexpr->cdr;
}
