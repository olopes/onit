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


/* think about cmocka's setup and teardown methods? */
void run_sparse_string_test(struct sparser_test_params * test_params) {
    /* arrange */
    struct sparse_ctx ctx = {NULL, L' ', L'"', NULL};
    struct sobj * sobj = NULL;
    int retval;
    FGET_CALLS = 0;
    TEST_STREAM = test_params->stream;

    /* act */
    retval = sparse_string(&ctx, &sobj);
    
    /* assert */
    assert_int_equal(test_params->return_value, retval);
    assert_int_equal(test_params->expected_fgetc_calls, FGET_CALLS);
    if(retval == SPARSE_OK) {
        assert_int_equal(0, wcscmp(sobj->data, test_params->expected));
        assert_int_equal(T_STRING, sobj->type);
        
        free(sobj->data);
        sobj_free(sobj);
    }
    
}


void sparse_string_should_create_sobj_with_str(void ** param) {
    (void) param; /* unused */
    struct sparser_test_params test_params = {L"HEY!\"", L"HEY!", SPARSE_OK, 5};
    
    run_sparse_string_test(&test_params);
    
}

void sparse_string_should_return_SPARSE_EOF_if_EOF_found(void ** param) {
    (void) param; /* unused */
    struct sparser_test_params test_params = {L"HE", NULL, SPARSE_EOF, 3};
    
    run_sparse_string_test(&test_params);
        
}

void sparse_string_should_handle_escape_chars(void ** param) {
    (void) param; /* unused */
    struct sparser_test_params test_params = {L"\\r\\n\\'\\\"\"", L"\r\n'\"", SPARSE_OK, 9};
    
    run_sparse_string_test(&test_params);
    
}

void sparse_string_should_handle_escaped_unicode_hex_chars(void ** param) {
    (void) param; /* unused */
    int i;
    struct sparser_test_params tests[6] = {
        {L"\\u0059\"", L"Y", SPARSE_OK, 7},
        {L"\\u65\"", L"e", SPARSE_OK, 5},
        {L"\\u073\"", L"s", SPARSE_OK, 6},
        {L"\\ua\"", L"\n", SPARSE_OK, 4},
        {L"\\u00211\"", L"!1", SPARSE_OK, 8},
        {L"\\u0059\\u65\\u073\\u00211\\ua\\u6e\\u6F\"  ", L"Yes!1\nno", SPARSE_OK, 34},
    };
    
    for (i = 0; i < 6; i++) {
        run_sparse_string_test(tests+i);
    }
}


void sparse_string_should_handle_escaped_oct_chars(void ** param) {
    (void) param; /* unused */
    int i;
    struct sparser_test_params tests[6] = {
        {L"\\131\"", L"Y", SPARSE_OK, 5},
        {L"\\145\"", L"e", SPARSE_OK, 5},
        {L"\\163\"", L"s", SPARSE_OK, 5},
        {L"\\41\"", L"!",  SPARSE_OK, 4},
        {L"\\0411\"", L"!1", SPARSE_OK, 6},
        {L"\\131\\145\\163\\0411\\41\\1\"  ", L"Yes!1!\1", SPARSE_OK, 23},
    };
    
    for (i = 0; i < 6; i++) {
        run_sparse_string_test(tests+i);
    }
}

void sparse_string_should_return_SPARSE_BAD_CHAR(void ** param) {
    (void) param; /* unused */
    int i;
    struct sparser_test_params tests[3] = {
        {L"\\x\"", L"-", SPARSE_BAD_CHAR, 2},
        {L"\\E\"", L"-", SPARSE_BAD_CHAR, 2},
        {L"\\ux\"", L"-", SPARSE_BAD_CHAR, 3},
    };
    
    for (i = 0; i < 3; i++) {
        run_sparse_string_test(tests+i);
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
