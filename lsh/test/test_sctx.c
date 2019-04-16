/* prod code includes */
#include "test_definitions.h"
#include "sctx.h"

static char * arguments[2] = {
    "arg1",
    NULL
};
static char * environment[4] = {
    "VAR1=VAL1",
    "PATH=/bin:/usr/bin",
    "HOME=/home/user",
    NULL
};
    
void sctx_do_nothing(void ** param)
{
    struct sctx * sctx;
    sctx = create_new_sctx(arguments, environment);
    release_sctx(sctx);
    assert_true(1);
}

void sctx_register_new_symbol(void ** param)
{
    struct sctx * sctx;
    struct sexpression * key;
    struct sexpression * value;
    struct mem_reference reference;
    
    sctx = create_new_sctx(arguments, environment);
    
    /* add the new symbol */
    create_stack_reference(sctx, L"VAR_NAME", 8, &reference);
    *reference.value = value = alloc_new_value(sctx, L"THE VALUE", 9);

    
    /* fetch the value using a different name */
    key = alloc_new_value(sctx, L"VAR_NAME", 8);
    
    assert_ptr_equal(value, lookup_name(sctx, key ));
    
    release_sctx(sctx);
}

void sctx_enter_namespace_register_new_symbol_leave_namespace_and_gc(void ** param)
{
    (void) param; /* unused */
    /* void * sctx = *param; */
    assert_true(1);
}


/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** state)
{
    
    *state = create_new_sctx(arguments, environment);

    return 0;
}

int teardown (void ** state)
{

    release_sctx(*state);
    
    return 0;
}

int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test (sctx_do_nothing),
        cmocka_unit_test (sctx_register_new_symbol),
        cmocka_unit_test_setup_teardown (sctx_enter_namespace_register_new_symbol_leave_namespace_and_gc, setup, teardown), 
    };
    /* disable stdout buffering */
    setbuf(stderr, NULL);
    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests_name (__FILE__, tests, NULL, NULL);

    return count_fail_tests;
}
