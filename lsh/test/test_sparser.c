/* prod code includes */
#include "sparser.h"

#include "mock_io.c"


void sparse_should_do_something_not_specified_yet(void ** param) {
    struct sexpression * actual_object;
    FILE dummy;
    
    mock_io(L"\"HEY!\"", 6);
    
    assert_int_equal(sparse(&dummy, &actual_object), SPARSE_OK);
    
    /* assert actual_object is a string? */
    assert_int_equal(0, wcscmp(actual_object->data, L"HEY!"));

    sexpr_free(actual_object);
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
        cmocka_unit_test (sparse_should_do_something_not_specified_yet),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
