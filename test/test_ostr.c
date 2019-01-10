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
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
