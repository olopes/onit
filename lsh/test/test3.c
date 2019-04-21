#include "test_definitions.h"

UnitTest(null_test_pass) {
    assert_true(1);
}

UnitTest(null_test_skip) {
	skip();
	fail_msg("should have been skipped %s", __FILE__);
}

UnitTest(null_test_failure) {
	skip();
	fail();
}

BeforeAll(setup)
{
    return 0;
}

AfterAll(teardown)
{
    return 0;
}

