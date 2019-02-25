/* prod code includes */
#include "sparser.h"
#include "sparser_privates.h"
#include "wcstr_sparser_adapter.h"


/* think about cmocka's setup and teardown methods? */
void run_sparse_symbol_test(struct sparser_test_params * test_params) {
    /* arrange */
    struct sparse_ctx ctx;
    struct sexpression * sobj = NULL;
    int return_value;
    
    ctx.stream = create_sparser_stream(WCSTR_ADAPTER, test_params->stream+1, wcslen(test_params->stream)-1);
    ctx.prev = L' ';
    ctx.next = *test_params->stream;
    
    /* act */
    return_value = sparse_symbol(&ctx, &sobj);
    
    /* assert */
    assert_int_equal(test_params->return_value, return_value);
    if(return_value == SPARSE_OK) {
        assert_int_equal(0, wcscmp(sobj->data, test_params->expected));
        /* FIXME define a sexpr_hint() function? */
        assert_int_equal(SC_SYMBOL, sobj->content);
        
        sexpr_free(sobj);
    }
    
    release_sparser_stream(ctx.stream);
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
        run_sparse_symbol_test(test_params+i);
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
