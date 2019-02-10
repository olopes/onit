#ifndef ASSERT_SEXPR
#define ASSERT_SEXPR
#include "sexpr.h"


void assert_sexpr_equal(struct sexpression * a, struct sexpression * b) {
    if(!sexpr_equal(a, b)) {
        fail_msg("%s", "S-Expressions are different");
    }
}

#endif 
