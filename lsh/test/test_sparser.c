/* prod code includes */
#include "sparser.h"

#include "assert_sexpr.c"
#include "mock_io.c"

FILE * dummy;

void sparse_should_do_parse_string_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    
    mock_io(L"\"HEY!\"");
    expected_object = sexpr_create_cstr(L"HEY!");
    
    assert_int_equal(sparse(dummy, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
}

void sparse_should_do_parse_symbol_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    
    mock_io(L" HEY!  ");
    expected_object = sexpr_create_cstr(L"HEY!");
    
    assert_int_equal(sparse(dummy, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
}

void sparse_should_do_parse_list_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    
    /*mock_io(L"(hey |y-o-u| \"check \\u34them\\\" dubs\" 123)");*/
    mock_io(L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)");
    expected_object = sexpr_cons(sexpr_create_cstr(L"hey"), 
                            sexpr_cons(sexpr_create_cstr(L"y-o-u"),
                                sexpr_cons(sexpr_create_cstr(L"check them dubs"),
                                    sexpr_cons(sexpr_create_cstr(L"123"),NULL))));
    
    assert_int_equal(sparse(dummy, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
}

void sparse_should_do_parse_pair_and_return_ok(void ** param) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    
    mock_io(L"(left . right)");
    expected_object = sexpr_cons(sexpr_create_cstr(L"left"), sexpr_create_cstr(L"right"));
    
    assert_int_equal(sparse(dummy, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
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
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
