/* prod code includes */
#include "sparser.h"
#include "sparser_privates.h"
#include "wcstr_sparser_adapter.h"

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
    struct sparse_ctx ctx;
    struct sexpression * sobj = pdummy;
    int retval;
    wchar_t * data = L" \r\n\t\f";
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    
    retval = sparse_object(&ctx, &sobj);
    
    /* EOF found and sobj set to NULL */
    assert_int_equal(SPARSE_EOF, retval);
    assert_null(sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_call_sparse_string(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L"\"X\"");
    
    expect_function_call(sparse_string);
    will_return(sparse_string, pdummy);
    will_return(sparse_string, 1);
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(1, retval);
    assert_ptr_equal(pdummy, sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_call_sparse_symbol(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L"|X|");
    
    expect_function_call(sparse_symbol);
    will_return(sparse_symbol, pdummy);
    will_return(sparse_symbol, 2);
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(2, retval);
    assert_ptr_equal(pdummy, sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_call_sparse_list(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L"(X)");
    
    expect_function_call(sparse_cons);
    will_return(sparse_cons, pdummy);
    will_return(sparse_cons, 3);
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(3, retval);
    assert_ptr_equal(pdummy, sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_call_sparse_quote(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L"'(X)");
    
    expect_function_call(sparse_quote);
    will_return(sparse_quote, pdummy);
    will_return(sparse_quote, 4);
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(4, retval);
    assert_ptr_equal(pdummy, sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_call_sparse_simple_symbol(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L"X");
    
    expect_function_call(sparse_symbol);
    will_return(sparse_symbol, pdummy);
    will_return(sparse_symbol, SPARSE_DOT_SYM);
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(SPARSE_DOT_SYM, retval);
    assert_ptr_equal(pdummy, sobj);
    
    release_sparser_stream(ctx.stream);
}

void sparse_object_should_return_paren_when_next_input_char_is_close_paren(void ** param) {
    struct sparse_ctx ctx = {NULL, L' ', L'"'};
    struct sexpression * sobj = NULL;
    int retval;
    wchar_t *data = (L")");
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, data, wcslen(data));
    ctx.prev = L' ';
    ctx.next = L'"';
    
    retval = sparse_object(&ctx, &sobj);
    
    /* must return the value and sobj returned by the called function */
    assert_int_equal(SPARSE_PAREN, retval);
    assert_ptr_equal(NULL, sobj);
    
    release_sparser_stream(ctx.stream);
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
        cmocka_unit_test (sparse_object_should_return_paren_when_next_input_char_is_close_paren),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
