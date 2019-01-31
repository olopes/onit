/* prod code includes */
#include "sparser.h"

#include "mock_io.c"

/* borrow definitions from sparse.c */
struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj);

/* mocks and stubs */
int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}


/* dummy good pointer */
struct sexpression dummy;
struct sexpression * pdummy = &dummy;

/*
what  should happen?
These are the requirements:

    () -> nil
    (x) -> cons(symbol(x), nil)
    (x . y) -> cons(symbol(x), symbol(y))
    (x . y . z) -> error
    (x y z) -> cons(symbol(x), cons(symbol(y), cons(symbol(z), nil)))
    create hint with list size
*/



void sparse_cons_should_return_null_when_input_is_empty_paren(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * obj;
    int return_value;
    mock_io(L"( )", 3);
    
    return_value = sparse_cons(&ctx, &obj);
    
    assert_null(obj);
    assert_int_equal(return_value, SPARSE_OK);
}

void sparse_cons_should_return_simplest_list_when_input_is_single_value(void ** param) {
    fail_msg("not implemented");
}

void sparse_cons_should_return_pair_when_input_has_two_values_separated_by_dot(void ** param) {
    fail_msg("not implemented");
}

void sparse_cons_should_return_error_when_input_has_multiple_dots(void ** param) {
    fail_msg("not implemented");
}

void sparse_cons_should_return_list_when_input_has_multiple_values(void ** param) {
    fail_msg("not implemented");
}

void sparse_cons_should_create_size_hints_on_parsed_value(void ** param) {
    fail_msg("not implemented");
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
        cmocka_unit_test (sparse_cons_should_return_null_when_input_is_empty_paren),
        cmocka_unit_test (sparse_cons_should_return_simplest_list_when_input_is_single_value),
        cmocka_unit_test (sparse_cons_should_return_pair_when_input_has_two_values_separated_by_dot),
        cmocka_unit_test (sparse_cons_should_return_error_when_input_has_multiple_dots),
        cmocka_unit_test (sparse_cons_should_return_list_when_input_has_multiple_values),
        cmocka_unit_test (sparse_cons_should_create_size_hints_on_parsed_value),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
