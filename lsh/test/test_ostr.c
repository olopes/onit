/* prod code includes */
#include <wctype.h>
#include <wchar.h>
#include "ostr.h"

void create_and_destroy_null_string(void ** param) 
{
    (void) param; /* unused */
    struct ostr * test_ostr;
    
    test_ostr = ostr_new(NULL);
    
    /* check new string contents */
    assert_int_equal( 0, test_ostr->length);
    assert_int_equal( 0, test_ostr->first->length);
    assert_int_equal(64, test_ostr->first->size); /* depends on constant PREALLOC_SIZE */
    assert_null(test_ostr->first->next);
    
    ostr_destroy(test_ostr);
}

void create_and_destroy_real_string(void ** param) 
{
    (void) param; /* unused */
    struct ostr * test_ostr;
    
    test_ostr = ostr_new(L"TEST STRING!");
    
    /* check new string contents */
    assert_int_equal(12, test_ostr->length);
    assert_int_equal(12, test_ostr->first->length);
    assert_int_equal(12, test_ostr->first->size); /* depends on string parameter */
    assert_null(test_ostr->first->next);
    
    ostr_destroy(test_ostr);
}

void ostr_destroy_should_free_complex_string(void ** param) 
{
    (void) param; /* unused */
    struct ostr * test_ostr;
    
    /* manually alloc string */
    test_ostr = (struct ostr *) test_malloc(sizeof(struct ostr));
    test_ostr->first = (struct ostr_token *) test_malloc(sizeof(struct ostr_token));
    test_ostr->first->str = (wchar_t *) test_malloc(sizeof(wchar_t));
    test_ostr->first->next = (struct ostr_token *) test_malloc(sizeof(struct ostr_token));
    test_ostr->first->next->str = (wchar_t *) test_malloc(sizeof(wchar_t));
    test_ostr->first->next->next = (struct ostr_token *) test_malloc(sizeof(struct ostr_token));
    test_ostr->first->next->next->str = (wchar_t *) test_malloc(sizeof(wchar_t));
    test_ostr->first->next->next->next = (struct ostr_token *) test_malloc(sizeof(struct ostr_token));
    test_ostr->first->next->next->next->str = (wchar_t *) test_malloc(sizeof(wchar_t));
    test_ostr->first->next->next->next->next = NULL;
    
    /* destroy string */
    ostr_destroy(test_ostr);

    /* test passes if all allocated blocks are freed */
}

void ostr_append_should_add_new_char_and_preserve_size(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    struct ostr_token test_token;
    struct ostr * result;
    wchar_t strang [64];
    
    test_ostr.first = test_ostr.last = &test_token;
    test_token.next = NULL;
    test_token.str = strang;
    test_ostr.length = test_token.length = 0;
    test_token.size = 64;
    
    result = ostr_append(&test_ostr, L'X');
    
    assert_int_equal( 1, test_ostr.length);
    assert_int_equal( 1, test_token.length);
    assert_int_equal(64, test_token.size);
    assert_int_equal(L'X', strang[0]);
    assert_null(test_token.next);
    assert_ptr_equal(result, &test_ostr);
    
}

void ostr_append_should_add_new_char_and_alloc_new_token(void ** param) 
{
    (void) param; /* unused */
    struct ostr * test_ostr;
    struct ostr * result;
    
    test_ostr = ostr_new(L"X");
    
    result = ostr_append(test_ostr, L'Y');
    
    assert_int_equal( 2, result->length);
    assert_int_equal( 1, result->first->length);
    assert_int_equal( 1, result->first->next->length);
    assert_int_equal(L'X', result->first->str[0]);
    assert_int_equal(L'Y', result->first->next->str[0]);
    assert_ptr_not_equal(result->first, result->last);
    assert_ptr_equal(result->last, result->first->next);
    
    ostr_destroy(result);
}


void ostr_replace_last_should_replace_last_char(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    struct ostr_token test_token;
    wchar_t strang [64];
    
    test_ostr.first = test_ostr.last = &test_token;
    test_token.next = NULL;
    test_token.str = strang;
    test_ostr.length = test_token.length = 7;
    test_token.size = 64;
    strang[6] = L'X';
    
    assert_non_null(ostr_replace_last(&test_ostr, L'Y'));
    
    assert_int_equal( 7, test_ostr.length);
    assert_int_equal(L'Y', strang[6]);

}

void ostr_replace_last_should_return_null_if_len_zero(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    test_ostr.length = 0;

    assert_null(ostr_replace_last(&test_ostr, L'X'));
}

void ostr_length_should_return_length(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    test_ostr.length = 100;

    assert_int_equal(100, ostr_length(&test_ostr));
}


void ostr_str_should_create_new_c_string(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    wchar_t * result_str = L"HELLO!";
    wchar_t * result;
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    
    result = ostr_str(&test_ostr);
    
    assert_int_equal( 6, wcslen(result));
    assert_int_equal( 0, wcsncmp(result_str, result, 7));

    free(result);
    
}

void ostr_dup_should_create_new_string_from_original_string(void ** param) 
{
    (void) param; /* unused */
    struct ostr * result;
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    wchar_t * result_str = L"HELLO!";
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    
    result = ostr_dup(&test_ostr);
    
    assert_int_equal( 6, result->length);
    assert_int_equal( 6, result->first->length);
    assert_int_equal( 0, wcsncmp(result_str, result->first->str, 7));

    
    ostr_destroy(result);
}

void ostr_concat_should_create_new_string_from_two_strings(void ** param) 
{
    (void) param; /* unused */
    struct ostr * result;
    struct ostr test_ostr1;
    struct ostr test_ostr2;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    
    test_ostr1.first = test_ostr1.last = &test_token1;
    test_ostr2.first = test_ostr2.last = &test_token2;
    test_token1.next = NULL;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr1.length = test_ostr2.length = 3;
    test_token1.length = test_token1.size = 3;
    test_token2.length = test_token2.size = 3;
    
    
    result = ostr_concat(&test_ostr1, &test_ostr2);
    
    assert_int_equal( 6, result->length);
    assert_int_equal( 3, result->first->length);
    assert_int_equal( 3, result->last->length);
    assert_int_equal( 0, wcsncmp(strang1, result->first->str, 7));
    assert_int_equal( 0, wcsncmp(strang2, result->last->str, 7));
    
    ostr_destroy(result);
}


void ostr_compact_should_reorganize_string_internals(void ** param) 
{
    (void) param; /* unused */
    struct ostr * result;
    struct ostr test_ostr1;
    struct ostr test_ostr2;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    wchar_t * result_str = L"HELLO!";
    
    test_ostr1.first = test_ostr1.last = &test_token1;
    test_ostr2.first = test_ostr2.last = &test_token2;
    test_token1.next = NULL;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr1.length = test_ostr2.length = 3;
    test_token1.length = test_token1.size = 3;
    test_token2.length = test_token2.size = 3;
    
    
    /* ostr_compact works with allocated strings, 
       so we use ostr_concat to create one */
    result = ostr_concat(&test_ostr1, &test_ostr2);
    
    /* the string contents are modified by this function */
    ostr_compact(result);
    
    assert_int_equal( 6, result->length);
    assert_int_equal( 0, wcsncmp(result_str, result->first->str, 7));
    assert_ptr_equal(result->first, result->last);
    assert_null(result->first->next);

    ostr_destroy(result);
    
}

wchar_t * TEST_STR;
FILE * TEST_STREAM;
int TEST_CALLS;
wint_t 
__wrap_fputwc(wchar_t wc, FILE * stream) {
    assert_int_equal(*TEST_STR, wc);
    
    assert_ptr_equal(TEST_STREAM, stream);
    
    TEST_STR++;
    TEST_CALLS++;
    return 1;
}


void ostr_puts_should_write_contents_to_stdout(void ** param) 
{
    (void) param; /* unused */
    wchar_t * data = L"HELLO!";
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    size_t written;
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    TEST_STR = data;
    TEST_STREAM = stdout;
    TEST_CALLS = 0;
    
    
    written = ostr_puts(&test_ostr);
    
    
    assert_int_equal(6, written);
    assert_int_equal(6, TEST_CALLS);
    
    /* other assertions performed by the wrapper function */
   
}

void ostr_fputs_should_write_contents_to_stderr(void ** param) 
{
    (void) param; /* unused */
    wchar_t * data = L"COOL!!";
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"COO";
    wchar_t * strang2 = L"L!!";
    size_t written;
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    TEST_STR = data;
    TEST_STREAM = stderr;
    TEST_CALLS = 0;
    
    
    written = ostr_fputs(&test_ostr, stderr);
    
    
    assert_int_equal(6, written);
    assert_int_equal(6, TEST_CALLS);
    
    /* other assertions performed by the wrapper function */
   
}

void ostr_char_at_should_return_nth_char(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    wint_t result1;
    wint_t result2;
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    result1 = ostr_char_at(&test_ostr, 1);
    result2 = ostr_char_at(&test_ostr, 4);
    
    assert_true(L'E' == result1);
    assert_true(L'O' == result2);
    
}

void ostr_char_at_should_return_WEOF_when_out_of_bounds(void ** param) 
{
    (void) param; /* unused */
    struct ostr test_ostr;
    struct ostr_token test_token1;
    struct ostr_token test_token2;
    wchar_t * strang1 = L"HEL";
    wchar_t * strang2 = L"LO!";
    wint_t result;
    
    test_ostr.first = &test_token1;
    test_ostr.last = test_token1.next = &test_token2;
    test_token2.next = NULL;
    test_token1.str = strang1;
    test_token2.str = strang2;
    test_ostr.length = 6;
    test_token1.length = test_token2.length = 3;
    test_token1.size = test_token2.size = 3;
    
    result = ostr_char_at(&test_ostr, 8);
    
    assert_true(WEOF == result);
    
}

/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** param)
{
    (void) param; /* unused */
    return 0;
}

int teardown (void ** param)
{
    (void) param; /* unused */
    return 0;
}


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test (create_and_destroy_null_string),
        cmocka_unit_test (create_and_destroy_real_string),
        cmocka_unit_test (ostr_destroy_should_free_complex_string),
        cmocka_unit_test (ostr_append_should_add_new_char_and_preserve_size),
        cmocka_unit_test (ostr_append_should_add_new_char_and_alloc_new_token),
        cmocka_unit_test (ostr_replace_last_should_replace_last_char),
        cmocka_unit_test (ostr_replace_last_should_return_null_if_len_zero),
        cmocka_unit_test (ostr_length_should_return_length),
        cmocka_unit_test (ostr_str_should_create_new_c_string),
        cmocka_unit_test (ostr_dup_should_create_new_string_from_original_string),
        cmocka_unit_test (ostr_concat_should_create_new_string_from_two_strings),
        cmocka_unit_test (ostr_compact_should_reorganize_string_internals),
        cmocka_unit_test (ostr_puts_should_write_contents_to_stdout),
        cmocka_unit_test (ostr_fputs_should_write_contents_to_stderr),
        cmocka_unit_test (ostr_char_at_should_return_nth_char),
        cmocka_unit_test (ostr_char_at_should_return_WEOF_when_out_of_bounds),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
