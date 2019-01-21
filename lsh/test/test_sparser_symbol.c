/* prod code includes */
#include "sobj.h"
#include "sparser.h"

wchar_t * TEST_STREAM;
int FGET_CALLS;
int UNGET_CALLS;

wint_t
__wrap_fgetwc(FILE * stream)
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
        
wint_t
__wrap_ungetwc(wint_t chr, FILE * stream)
{
    UNGET_CALLS++;
    
    assert_int_equal(chr, *(TEST_STREAM-1));
    
    return chr;
}
        

struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int 
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj);


/* think about cmocka's setup and teardown methods? */
void run_sparse_symbol_test(struct sparser_test_params * test_params) {
    /* arrange */
    struct sparse_ctx ctx = {NULL, L' ', L' ', NULL};
    struct sobj * sobj = NULL;
    int retval;
    /* consume the first char like sparse_object() would do */
    ctx.next = *test_params->stream;
    TEST_STREAM = test_params->stream+1;
    FGET_CALLS = 0;
    
    /* act */
    retval = sparse_symbol(&ctx, &sobj);
    
    /* assert */
    assert_int_equal(test_params->return_value, retval);
    assert_int_equal(test_params->expected_fgetc_calls, FGET_CALLS);
    if(retval == SPARSE_OK) {
        assert_int_equal(0, wcscmp(sobj->data, test_params->expected));
        assert_int_equal(T_SYMBOL, sobj->type);
        
        free(sobj->data);
        sobj_free(sobj);
    }
    
}

/* 
 * Symbol input definition based on
 * http://docs.racket-lang.org/guide/symbols.html
 */

void sparse_symbol_should_accept_any_valid_character(void ** param) 
{
    (void) param; /* unused */;
    int i;
    struct sparser_test_params test_params[4] = {
        {L"xyz", L"xyz", SPARSE_OK, 3},
        {L"x", L"x", SPARSE_OK, 1},
        {L"çá", L"çá", SPARSE_OK, 2},
        {L"x.y.z", L"x.y.z", SPARSE_OK, 5},
    };
    
    for (i = 0; i < 4; i++) {
        run_sparse_symbol_test(test_params+i);
    }
}

void sparse_symbol_should_accept_invalid_character_if_escaped(void ** param) 
{
    (void) param; /* unused */;
    int i;
    struct sparser_test_params test_params[2] = {
        {L"x\\ yz", L"x yz", SPARSE_OK, 5},
        {L"\\(\\)\\[\\]\\{\\}\\\"\\,\\'\\`\\;\\#\\|\\\\", L"()[]{}\",'`;#|\\", SPARSE_OK, 28},
    };
    
    for (i = 0; i < 2; i++) {
        run_sparse_symbol_test(test_params+i);
    }
}

void sparse_symbol_should_return_BAD_SYMBOL_if_no_char_present_after_escaping_slash(void ** param) 
{
    (void) param; /* unused */;
    struct sparser_test_params test_params = {L"x\\", NULL, SPARSE_BAD_SYM, 2};
    
    run_sparse_symbol_test(&test_params);
}

void sparse_symbol_should_return_BAD_SYMBOL_if_hash_used_at_start_of_symbol(void ** param) 
{
    (void) param; /* unused */;
    struct sparser_test_params test_params = {L"#hash", NULL, SPARSE_BAD_SYM, 1};
    
    run_sparse_symbol_test(&test_params);
}

void sparse_symbol_should_return_DOT_SYMBOL_if_value_is_single_dot(void ** param) 
{
    (void) param; /* unused */;
    struct sparser_test_params test_params = {L".", NULL, SPARSE_DOT_SYM, 1};
    
    run_sparse_symbol_test(&test_params);
}

void sparse_symbol_should_accept_special_chars_if_quoted_with_pipe(void ** param) 
{
    (void) param; /* unused */;
    struct sparser_test_params test_params = {L"x|( )|y ", L"x( )y", SPARSE_OK, 7};
    
    run_sparse_symbol_test(&test_params);
}

void sparse_symbol_should_return_BAD_SYMBOL_if_quoting_pipe_is_not_closed(void ** param) 
{
    (void) param; /* unused */;
    struct sparser_test_params test_params = {L"x|y", NULL, SPARSE_BAD_SYM, 3};
    
    run_sparse_symbol_test(&test_params);
}

void sparse_symbol_should_put_char_back_into_stream_and_stop_if_char_is_special(void ** param) 
{
    (void) param; /* unused */;
    int i;
    struct sparser_test_params test_params[] = {
        {L"xyz ", L"xyz", SPARSE_OK, 3},
        {L"xyz(.)", L"xyz", SPARSE_OK, 3},
        {L"xyz'a", L"xyz", SPARSE_OK, 3},
    };
    
    for (i = 0; i < 3; i++) {
        UNGET_CALLS = 0;
        run_sparse_symbol_test(test_params+i);
        assert_int_equal(1,UNGET_CALLS);
    }
    
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
        cmocka_unit_test (sparse_symbol_should_accept_any_valid_character),
        cmocka_unit_test (sparse_symbol_should_accept_invalid_character_if_escaped),
        cmocka_unit_test (sparse_symbol_should_return_BAD_SYMBOL_if_no_char_present_after_escaping_slash),
        cmocka_unit_test (sparse_symbol_should_return_BAD_SYMBOL_if_hash_used_at_start_of_symbol),
        cmocka_unit_test (sparse_symbol_should_return_DOT_SYMBOL_if_value_is_single_dot),
        cmocka_unit_test (sparse_symbol_should_accept_special_chars_if_quoted_with_pipe),
        cmocka_unit_test (sparse_symbol_should_return_BAD_SYMBOL_if_quoting_pipe_is_not_closed),
        cmocka_unit_test (sparse_symbol_should_put_char_back_into_stream_and_stop_if_char_is_special),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
