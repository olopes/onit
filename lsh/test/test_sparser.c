/* prod code includes */
#include <cmocka.h>
#include <string.h>
#include "sparser.h"
#include "cstr_sparser_adapter.h"
#include "wcstr_sparser_adapter.h"
#include "stdio_sparser_adapter.h"
#include "assert_sexpr.c"

void sparse_should_do_parse_string_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    wchar_t * data = L"\"HEY!\"";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    
    expected_object = sexpr_create_cstr(L"HEY!");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

void sparse_should_do_parse_symbol_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    wchar_t * data = L" HEY!  ";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_create_cstr(L"HEY!");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

void sparse_should_do_parse_list_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    /* wchar_t * data = L"(hey |y-o-u| \"check \\u34them\\\" dubs\" 123)"; */
    wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_cons(sexpr_create_cstr(L"hey"), 
                            sexpr_cons(sexpr_create_cstr(L"y-o-u"),
                                sexpr_cons(sexpr_create_cstr(L"check them dubs"),
                                    sexpr_cons(sexpr_create_cstr(L"123"),NULL))));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

void sparse_should_do_parse_pair_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    char * data = "(left . right)";
    
    stream = create_sparser_stream(CSTR_ADAPTER, data, strlen(data));
    expected_object = sexpr_cons(sexpr_create_cstr(L"left"), sexpr_create_cstr(L"right"));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

void sparse_should_do_parse_a_list_from_a_file(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    char * filename = "test/sexpression.lsh"; /* (abc |d.e-f| "ghi" 1/3+4i)  */
    
    
    stream = create_sparser_stream(FILE_ADAPTER, filename);
    expected_object = sexpr_cons(sexpr_create_cstr(L"abc"), 
                            sexpr_cons(sexpr_create_cstr(L"d.e-f"),
                                sexpr_cons(sexpr_create_cstr(L"ghi"),
                                    sexpr_cons(sexpr_create_cstr(L"1/3+4i"),NULL))));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

void sparse_should_parse_multiple_objects_from_a_stream(void ** param) {
    struct sexpression * actual_object;
    struct sexpression * expected_objects[4];
    struct sexpression ** expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"symbol\"glued string\"\n(hey |y-o-u| 123)      ";
    
    /* wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)"; */
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_objects[0] = sexpr_create_cstr(L"symbol");
    expected_objects[1] = sexpr_create_cstr(L"glued string");
    expected_objects[2] = sexpr_cons(sexpr_create_cstr(L"hey"), 
                            sexpr_cons(sexpr_create_cstr(L"y-o-u"),
                                sexpr_cons(sexpr_create_cstr(L"123"),NULL)));
    expected_objects[3] = NULL;
    
    expected_object = expected_objects;
    while(*expected_object) {
        assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
        assert_sexpr_equal(*expected_object, actual_object);
        sexpr_free(actual_object);
        sexpr_free(*expected_object);
        expected_object++;
    }
    
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_EOF);
    assert_null(actual_object);
    
    release_sparser_stream(stream);
}

void sparse_should_ignore_comments(void ** param) {
    struct sexpression * actual_object;
    struct sexpression * expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"; single line comment\n#|multi\nline\ncomment|#\n2";
    
    /* wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)"; */
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_create_cstr(L"2");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    assert_sexpr_equal(expected_object, actual_object);
    sexpr_free(actual_object);
    sexpr_free(expected_object);

    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_EOF);
    assert_null(actual_object);
    
    release_sparser_stream(stream);
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
        cmocka_unit_test (sparse_should_do_parse_string_and_return_ok),
        cmocka_unit_test (sparse_should_do_parse_symbol_and_return_ok),
        cmocka_unit_test (sparse_should_do_parse_list_and_return_ok),
        cmocka_unit_test (sparse_should_do_parse_pair_and_return_ok),
        cmocka_unit_test (sparse_should_do_parse_a_list_from_a_file),
        cmocka_unit_test (sparse_should_parse_multiple_objects_from_a_stream),
        cmocka_unit_test (sparse_should_ignore_comments),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
