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
sparse_string(struct sparse_ctx * ctx, struct sobj ** obj);


void sparse_string_should_create_sobj_with_str(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    wchar_t * expected = L"HEY!";
    
    FGET_CALLS = 0;
    TEST_STREAM = L"HEY!\""; 
    
    retval = sparse_string(&ctx, &sobj);
    
    assert_int_equal(4, sobj->len);
    assert_int_equal(T_STRING, sobj->type);
    assert_int_equal(0, wcscmp(sobj->data, expected));
    assert_int_equal(5, FGET_CALLS);
    assert_true(ctx.next == L'"');
    assert_true(ctx.prev == L'!');
    assert_int_equal(SPARSE_OK, retval);
    
    free(sobj->data);
    sobj_free(sobj);
}

void sparse_string_should_return_SPARSE_EOF_if_EOF_found(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    
    FGET_CALLS = 0;
    TEST_STREAM = L"HE"; 
    sobj = NULL;
    
    retval = sparse_string(&ctx, &sobj);
    
    assert_int_equal(SPARSE_EOF, retval);
    
    assert_null(sobj);
    assert_int_equal(3, FGET_CALLS);
    assert_true(ctx.next == WEOF);
    assert_true(ctx.prev == L'E');
    
}

void sparse_string_should_handle_escape_chars(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj;
    int retval;
    wchar_t * expected = L"\r\n'\"";
    
    FGET_CALLS = 0;
    TEST_STREAM = L"\\r\\n\\'\\\"\""; 
    sobj = NULL;
    
    retval = sparse_string(&ctx, &sobj);
    
    assert_int_equal(SPARSE_OK, retval);
    assert_int_equal(0, wcscmp(sobj->data, expected));
    assert_int_equal(9, FGET_CALLS);

    free(sobj->data);
    sobj_free(sobj);
}

void sparse_string_should_handle_escaped_unicode_hex_chars(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj = NULL;
    struct parametrized_test_case tests[6] = {
        {L"\\u0059\"", L"Y", SPARSE_OK, 7},
        {L"\\u65\"", L"e", SPARSE_OK, 5},
        {L"\\u073\"", L"s", SPARSE_OK, 6},
        {L"\\ua\"", L"\n", SPARSE_OK, 4},
        {L"\\u00211\"", L"!1", SPARSE_OK, 8},
        {L"\\u0059\\u65\\u073\\u00211\\ua\\u6e\\u6F\"  ", L"Yes!1\nno", SPARSE_OK, 34},
    };
    int i;
    
    for (i = 0; i < 6; i++) {
        ctx.next = L'"';
        FGET_CALLS = 0;
        TEST_STREAM = tests[i].stream;
    
        assert_int_equal(tests[i].retval, sparse_string(&ctx, &sobj));
        assert_int_equal(0, wcscmp(sobj->data, tests[i].expected));
        assert_int_equal(tests[i].calls, FGET_CALLS);

        free(sobj->data);
        sobj_free(sobj);
    }
}


void sparse_string_should_handle_escaped_oct_chars(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj = NULL;
    struct parametrized_test_case tests[6] = {
        {L"\\131\"", L"Y", SPARSE_OK, 5},
        {L"\\145\"", L"e", SPARSE_OK, 5},
        {L"\\163\"", L"s", SPARSE_OK, 5},
        {L"\\41\"", L"!",  SPARSE_OK, 4},
        {L"\\0411\"", L"!1", SPARSE_OK, 6},
        {L"\\131\\145\\163\\0411\\41\\1\"  ", L"Yes!1!\1", SPARSE_OK, 23},
    };
    int i;
    
    for (i = 0; i < 6; i++) {
        ctx.next = L'"';
        FGET_CALLS = 0;
        TEST_STREAM = tests[i].stream;
    
        assert_int_equal(tests[i].retval, sparse_string(&ctx, &sobj));
        assert_int_equal(0, wcscmp(sobj->data, tests[i].expected));
        assert_int_equal(tests[i].calls, FGET_CALLS);

        free(sobj->data);
        sobj_free(sobj);
    }
}

void sparse_string_should_return_SPARSE_BAD_CHAR(void ** param) {
    (void) param; /* unused */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj = NULL;
    struct parametrized_test_case tests[3] = {
        {L"\\x\"", L"-", SPARSE_BAD_CHAR, 2},
        {L"\\E\"", L"-", SPARSE_BAD_CHAR, 2},
        {L"\\ux\"", L"-", SPARSE_BAD_CHAR, 3},
    };
    int i;
    
    for (i = 0; i < 3; i++) {
        ctx.next = L'"';
        FGET_CALLS = 0;
        TEST_STREAM = tests[i].stream;
    
        assert_int_equal(tests[i].retval, sparse_string(&ctx, &sobj));
        assert_int_equal(tests[i].calls, FGET_CALLS);

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
        cmocka_unit_test (sparse_string_should_create_sobj_with_str),
        cmocka_unit_test (sparse_string_should_return_SPARSE_EOF_if_EOF_found),
        cmocka_unit_test (sparse_string_should_handle_escape_chars),
        cmocka_unit_test (sparse_string_should_handle_escaped_unicode_hex_chars),
        cmocka_unit_test (sparse_string_should_handle_escaped_oct_chars),
        cmocka_unit_test (sparse_string_should_return_SPARSE_BAD_CHAR),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
