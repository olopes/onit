/* prod code includes */
#include "test_definitions.h"
#include "sctx.h"
#include "sctx_privates.h"

void sctx_do_nothing(void ** param)
{
    (void) param; /* unused */
    assert_true(1);
}

void sctx_register_new_symbol(void ** param)
{
    void * sctx = *param;
    struct sexpression * name1;
    struct sexpression * name2;
    struct sexpression * value;
    
    /* add the new symbol */
    name1 = sexpr_create_value(L"VAR_NAME", 8);
    value = alloc_new_value(sctx, L"THE VALUE", 9);
    register_value(sctx, name1, value);
    
    /* fetch the value using a different name */
    name2 = sexpr_create_value(L"VAR_NAME", 8);
    
    assert_ptr_equal(value, lookup_name(sctx, name2));
    
    sexpr_free(name1);
    sexpr_free(name2);
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
        /* cmocka_unit_test_setup_teardown (sctx_register_new_symbol, setup, teardown),
        cmocka_unit_test_setup_teardown (sctx_enter_namespace_register_new_symbol_leave_namespace_and_gc, setup, teardown), */
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, NULL, NULL);

    return count_fail_tests;
}
