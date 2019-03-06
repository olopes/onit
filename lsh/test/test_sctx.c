/* prod code includes */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <cmocka.h>
#include "sctx.h"
#include "sctx_privates.h"

void sctx_do_nothing(void ** param)
{
    (void) param; /* unused */
    assert_true(1);
}

/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** state)
{
    char * arguments[2] = {
        "arg1",
        NULL
    };
    char * environment[4] = {
        "VAR1=VAL1",
        "PATH=/bin:/usr/bin",
        "HOME=/home/user",
        NULL
    };
    
    *state = init_environment(arguments, environment);
    
    return 0;
}

int teardown (void ** state)
{
    
    release_environment(*state);
    
    return 0;
}


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test_setup_teardown (sctx_do_nothing, setup, teardown),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, NULL, NULL);

    return count_fail_tests;
}
