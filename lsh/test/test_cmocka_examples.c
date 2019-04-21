#include "test_definitions.h"

BeforeAll(sample_group_setup)
{
    (void) state; /* unused */
    print_message("Sample group setup performed\n");
    return 0;
}

AfterAll(sample_group_teardown)
{
    (void) state; /* unused */
    print_message("Sample group teardown performed\n");
    return 0;
}


static int sample_setup(void ** state) {
    (void) state; /* unused */
    print_message("Sample setup performed\n");
    return 0;
}

static int sample_teardown(void ** state) {
    (void) state; /* unused */
    print_message("Sample teardown performed\n");
    return 0;
}

UnitTest(null_test_pass) {
    assert_true(1);
}

UnitTest(null_test_setup, .setup=sample_setup) {
    assert_true(1);
}

UnitTest(null_test_teardown, .teardown=sample_teardown) {
    assert_true(1);
}

UnitTest(null_test_repeat, .repeat=3) {
    assert_true(1);
}

UnitTest(null_test_ignore, .ignore=1) {
   fail_msg("should have been ignored %s", __FILE__);
}

UnitTest(null_test_skip) {
   skip();
   fail_msg("should have been skipped %s", __FILE__);
}

UnitTest(null_test_failure) {
   skip(); /* skipped - otherwise the build fails */
   fail();
}

