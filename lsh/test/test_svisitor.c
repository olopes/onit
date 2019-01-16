/* prod code includes */
#include "svisitor.h"
#include "wraps.c"

/* mock implementations */
void svisitor(struct sobj * sobj, struct scallback * cb) {
    check_expected(sobj);
    check_expected(cb->context);
}

/* test case */
void dump_sobj_should_call_svisitor (void ** state) {
    struct sobj dummy_obj;
    FILE dummy_file;
    
    expect_value(svisitor, sobj, &dummy_obj);
    expect_value(svisitor, cb->context, &dummy_file);
    
    
    dump_sobj(&dummy_obj, &dummy_file);
    
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
        cmocka_unit_test (dump_sobj_should_call_svisitor),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}