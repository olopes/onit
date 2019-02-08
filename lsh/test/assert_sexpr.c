#ifndef ASSERT_SEXPR
#define ASSERT_SEXPR

#include "sexpr.h"

int sexpr_equal(struct sexpression * a, struct sexpression * b) {
    int are_equal;
    
    /* same object ? */
    if(a == b) {
        are_equal = 1;
    } else if(a == NULL || b == NULL) {
        are_equal = 0;
    } else if (sexpr_type(a) != sexpr_type(b)) {
        are_equal = 0;
    } else if (sexpr_is_cons(a)) {
        are_equal = (sexpr_equal(sexpr_car(a), sexpr_car(b)) && sexpr_equal(sexpr_cdr(a), sexpr_cdr(b)));
    } else {
        are_equal = wcsncmp((wchar_t *) a->data, (wchar_t *) b->data, a->len) == 0;
    }
    return are_equal;
}

void assert_sexpr_equal(struct sexpression * a, struct sexpression * b) {
    if(!sexpr_equal(a, b)) {
        fail_msg("%s", "S-Expressions are different");
    }
}

#endif 
