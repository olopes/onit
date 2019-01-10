/* TODO implement */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/* prod code includes */
#include "svisitor.h"
#include "wraps.c"

iswspace

int 
sparse_object(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_string(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_simple_symbol(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_quote(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_cons(struct sparse_ctx * ctx, struct sobj ** obj);


void null_test_failure(void ** param) {
    assert_true(1);
}


/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** state)
{
    return 0;
}

int teardown (void ** state)
{
    return 0;
}


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test (null_test_failure),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
