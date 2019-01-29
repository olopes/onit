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
int sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj);
int sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj);

/* mocks and stubs */
wchar_t * TEST_STREAM;
int FGET_CALLS;

wint_t __wrap_fgetwc(FILE * stream)
{
    wint_t chr;
    
    FGET_CALLS++;
    chr = *TEST_STREAM;
    
    if(chr) {
        TEST_STREAM++;
    } else {
        chr = WEOF;
    }
    return chr;
}

int 
sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}
int 
sparse_symbol(struct sparse_ctx * ctx, struct sexpression ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}
int 
sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}
int 
sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj) {
    function_called();
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}


/* dummy good pointer */
struct sexpression dummy;
struct sexpression * pdummy = &dummy;


void sparse_object_should_never_call_sparse_functions_when_input_is_space(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = pdummy;
    int retval;
    TEST_STREAM = L" \r\n\t\f";
    
    retval = sparse_object(&ctx, &sobj);
    
    /* EOF found and sobj set to NULL */
    assert_int_equal(SPARSE_EOF, retval);
    assert_null(sobj);
}

void sparse_object_should_call_sparse_string(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = NULL;
    int retval;
    TEST_STREAM = L"\"X\"";
    
    expect_function_call(sparse_string);
    will_return(sparse_string, pdummy);
    will_return(sparse_string, 1);
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(1, retval);
    assert_ptr_equal(pdummy, sobj);
}

void sparse_object_should_call_sparse_symbol(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = NULL;
    int retval;
    TEST_STREAM = L"|X|";
    
    expect_function_call(sparse_symbol);
    will_return(sparse_symbol, pdummy);
    will_return(sparse_symbol, 2);
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(2, retval);
    assert_ptr_equal(pdummy, sobj);
}

void sparse_object_should_call_sparse_list(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = NULL;
    int retval;
    TEST_STREAM = L"(X)";
    
    expect_function_call(sparse_cons);
    will_return(sparse_cons, pdummy);
    will_return(sparse_cons, 3);
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(3, retval);
    assert_ptr_equal(pdummy, sobj);
}

void sparse_object_should_call_sparse_quote(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = NULL;
    int retval;
    TEST_STREAM = L"'(X)";
    
    expect_function_call(sparse_quote);
    will_return(sparse_quote, pdummy);
    will_return(sparse_quote, 4);
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(4, retval);
    assert_ptr_equal(pdummy, sobj);
}

void sparse_object_should_call_sparse_simple_symbol(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sexpression * sobj = NULL;
    int retval;
    TEST_STREAM = L"X";
    
    expect_function_call(sparse_symbol);
    will_return(sparse_symbol, pdummy);
    will_return(sparse_symbol, 5);
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(5, retval);
    assert_ptr_equal(pdummy, sobj);
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
        cmocka_unit_test (sparse_object_should_never_call_sparse_functions_when_input_is_space),
        cmocka_unit_test (sparse_object_should_call_sparse_string),
        cmocka_unit_test (sparse_object_should_call_sparse_symbol),
        cmocka_unit_test (sparse_object_should_call_sparse_list),
        cmocka_unit_test (sparse_object_should_call_sparse_quote),
        cmocka_unit_test (sparse_object_should_call_sparse_simple_symbol),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
