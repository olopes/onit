/* prod code includes */
#include "sobj.h"
#include "sparser.h"

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
        

struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int 
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj);



/* 
 * Symbol input definition based on
 * http://docs.racket-lang.org/guide/symbols.html
 */

void sparse_symbol_should_accept_any_valid_character(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_accept_invalid_character_if_escaped(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_return_BAD_SYMBOL_if_no_char_present_after_escaping_slash(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_return_BAD_SYMBOL_if_hash_used_at_start_of_symbol(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_return_BAD_SYMBOL_if_value_is_single_dot(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_accept_special_chars_if_quoted_with_pipe(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_return_BAD_SYMBOL_if_quoting_pipe_is_not_closed(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
}

void sparse_symbol_should_put_char_back_into_stream_and_stop_if_char_is_special(void ** param) 
{
    (void) param; /* unused */;
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    fail_msg("Not implemented");
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
        cmocka_unit_test (sparse_symbol_should_return_BAD_SYMBOL_if_value_is_single_dot),
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
