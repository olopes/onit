/* TODO implement */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/* prod code includes */
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include "sparser.h"

/* borrow definitions from sparse.c */
struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};
int sparse_object(struct sparse_ctx * ctx, struct sobj ** obj);

/* mocks and stubs */
wint_t 
__wrap_fwgetc(FILE *fp) {
    return mock();
}


struct sparse_ctx mctx;
struct sobj * mobj;

int 
sparse_string(struct sparse_ctx * ctx, struct sobj ** obj) {
    check_expected(ctx);
    check_expected(obj);
    return mock();
}
int 
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj) {
    check_expected(ctx);
    check_expected(obj);
    return mock();
}
int 
sparse_simple_symbol(struct sparse_ctx * ctx, struct sobj ** obj) {
    check_expected(ctx);
    check_expected(obj);
    return mock();
}
int 
sparse_quote(struct sparse_ctx * ctx, struct sobj ** obj) {
    check_expected(ctx);
    check_expected(obj);
    return mock();
}
int 
sparse_cons(struct sparse_ctx * ctx, struct sobj ** obj) {
    check_expected(ctx);
    check_expected(obj);
    return mock();
}

/*

test iswspace 
test '"'
test '|'
test '('
test '''
test other

*/





void sparse_object_should_never_call_sparse_functions_when_input_is_space(void ** param) {
    
    will_return(__wrap_fwgetc, L' ');
    will_return(__wrap_fwgetc, L'\r');
    will_return(__wrap_fwgetc, L'\n');
    will_return(__wrap_fwgetc, L'\t');
    will_return(__wrap_fwgetc, L'\f');
    will_return(__wrap_fwgetc, WEOF);
    
    sparse_object(&mctx, &mobj);
    
}

void sparse_object_should_call_sparse_string(void ** param) {
    will_return(__wrap_fwgetc, L'"');
    
    expect_value(sparse_string, ctx, &mctx);
    expect_value(sparse_string, obj, &mobj);
    will_return(sparse_string, 0);
    
    sparse_object(&mctx, &mobj);
}

void sparse_object_should_call_sparse_symbol(void ** param) {
    will_return(__wrap_fwgetc, L'|');
    
    expect_value(sparse_symbol, ctx, &mctx);
    expect_value(sparse_symbol, obj, &mobj);
    will_return(sparse_symbol, 0);
    
    sparse_object(&mctx, &mobj);
}

void sparse_object_should_call_sparse_list(void ** param) {
    will_return(__wrap_fwgetc, L'(');
    
    expect_value(sparse_cons, ctx, &mctx);
    expect_value(sparse_cons, obj, &mobj);
    will_return(sparse_cons, 0);
    
    sparse_object(&mctx, &mobj);
}

void sparse_object_should_call_sparse_quote(void ** param) {
    will_return(__wrap_fwgetc, L'\'');
    
    expect_value(sparse_quote, ctx, &mctx);
    expect_value(sparse_quote, obj, &mobj);
    will_return(sparse_quote, 0);
    
    sparse_object(&mctx, &mobj);
}

void sparse_object_should_call_sparse_simple_symbol(void ** param) {
    will_return(__wrap_fwgetc, L'x');
    
    expect_value(sparse_simple_symbol, ctx, &mctx);
    expect_value(sparse_simple_symbol, obj, &mobj);
    will_return(sparse_simple_symbol, 0);
    
    sparse_object(&mctx, &mobj);
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
