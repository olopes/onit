/* prod code includes */
#include "stype.h"
#include "sobj.h"
#include "sexpr.h"
#include "sparser.h"

/* borrow definitions from sparse.c */
struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int sparse_object(struct sparse_ctx * ctx, struct sobj ** obj);
int sparse_quote(struct sparse_ctx * ctx, struct sobj ** obj);

/* mocks and stubs */
int 
sparse_object(struct sparse_ctx * ctx, struct sobj ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sobj *);
    return mock();
}


/* dummy good pointer */
struct sobj dummy;
struct sobj * pdummy = &dummy;


void sparse_quote_should_call_sparse_object_and_create_quote_list_if_success(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L' ', NULL};
    wchar_t *yes_str = L"YES";
    struct sobj * a_object;
    struct sobj * sobj;
    struct sstr * expected_quote;
    int retval;
    
    a_object = sobj_from_string(yes_str, 3);
    
    expect_function_call(sparse_string);
    will_return(sparse_object, a_object);
    will_return(sparse_object, SPARSE_OK);
    
    
    retval = sparse_quote(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(SPARSE_OK, retval);
    
    /* sparse_quote will produce the following list: (quote (a_object . NULL)) */
    expected_quote = sobj_to_symbol(sexpr_car(sobj_to_cons(sobj)));
    assert_true(wcsncmp(expected_quote->data, L"quote", 5) == 0);
    
    /* FIXME - my API is crappy because it's getting complicated to check stuff ? */
    assert_ptr_equal(a_object, sexpr_car(sobj_to_cons(sexpr_cdr(sobj_to_cons(sobj)))));
    
    assert_ptr_equal(NULL, sexpr_cdr(sobj_to_cons(sexpr_cdr(sobj_to_cons(sobj)))));
    
    sobj_free(sobj);
    
}

void sparse_quote_should_call_sparse_object_and_return_error_code(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L' ', NULL};
    struct sobj * sobj = NULL;
    int retval;
    
    expect_function_call(sparse_object);
    will_return(sparse_symbol, pdummy);
    will_return(sparse_symbol, SPARSE_BAD_CHAR);
    
    
    retval = sparse_quote(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(SPARSE_BAD_CHAR, retval);
    assert_ptr_equal(NULL, sobj);
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
        cmocka_unit_test (sparse_quote_should_call_sparse_object_and_create_quote_list_if_success),
        cmocka_unit_test (sparse_quote_should_call_sparse_object_and_return_error_code),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
