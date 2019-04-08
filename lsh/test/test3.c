#include <cmocka.h>
/* TODO implement */

void null_test_pass(void ** param) {
    assert_true(1);
}

void null_test_skip(void ** param) {
	skip();
	fail_msg("should have been skipped");
}

void null_test_failure(void ** param) {
	skip();
	fail();
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
    const struct CMUnitTest test3 [] =
    {
        cmocka_unit_test (null_test_pass),
        cmocka_unit_test (null_test_skip),
        cmocka_unit_test (null_test_failure),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests_name (__FILE__, test3, setup, teardown);

    return count_fail_tests;
}
