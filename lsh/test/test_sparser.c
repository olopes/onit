/* prod code includes */
#include "sparser.h"

#include "mock_io.c"

static struct sexpression * actual_object;

void sparse_should_do_something_not_specified_yet(void ** param) {
    FILE dummy;
    
    mock_io(L"\"HEY!\"", 6);
    
    assert_int_equal(sparse(&dummy, &actual_object), SPARSE_OK);
    
    /* assert actual_object is a string? */
    assert_int_equal(0, wcscmp(sobj->data, L"HEY!"));

    
}


/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** state)
{
    return 0;
}

int teardown (void ** state)
{
    
    sexpr_free(actual_object);
    
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
