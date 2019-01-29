/* prod code includes */
#include "sparser.h"

/* borrow definitions from sparse.c */
struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_symbol(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_simple_symbol(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj);


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
