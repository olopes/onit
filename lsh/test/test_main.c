#include "test_definitions.h"
#include <stdio.h>


/* injected automatically by the linker script */
extern struct LshUnitTest __start_lshtest;
extern struct LshUnitTest __stop_lshtest;
extern struct GroupSetupTeardown __start_lshsetup;
extern struct GroupSetupTeardown __stop_lshsetup;


static void discover_tests(void);
static void discover_group_setup_teardown(void);
static void discover_unit_tests(void);
static int _run_group_setup(void ** state);
static int _run_group_teardown(void ** state);


static struct CMUnitTest * tests = NULL;
static size_t num_tests = 0;

static struct GroupSetupTeardown * test_group_setup = NULL;
static size_t num_group_setup = 0;

/* "Auto" discover tests using default linker scripts */

static void discover_tests(void) {
    
    discover_group_setup_teardown();
    
    discover_unit_tests();
    
}

static void discover_group_setup_teardown(void) {
    struct GroupSetupTeardown * ptr;
    
    test_group_setup = &__start_lshsetup;

    for(ptr = test_group_setup, num_group_setup = 0; ptr < &__stop_lshsetup; ptr++, num_group_setup++);
    
}


static void discover_unit_tests(void) {
    int i;
    struct LshUnitTest * ptr;
    
    ptr = &__start_lshtest;
    for(num_tests = 0; ptr < &__stop_lshtest; ptr++, num_tests++);
    
    tests = (struct CMUnitTest *) malloc(sizeof(struct CMUnitTest) * num_tests);
    if(tests == NULL) {
        fprintf(stderr, "ERROR: Error allocating test case array\n");
        fflush(stderr);
        fflush(stdout);
        abort();
    }
    
    ptr = &__start_lshtest;
    for(i = 0; i < num_tests; i++) {
        tests[i] = ptr[i].test;
    }
    
}


static int _run_group_setup(void ** state) {
    size_t i;
    int retval;
    
    if(test_group_setup == NULL || num_group_setup == 0) {
        /* nothing to do */
        return 0;
    }
    
    for(i = 0; i < num_group_setup; i++) {
        if(test_group_setup[i].setup == NULL) {
            continue;
        }
        retval = test_group_setup[i].setup(state);
        if(retval) {
            return retval;
        }
    }
    
    return 0;
    
}
    
static int _run_group_teardown(void ** state) {
    size_t i;
    int retval;
    
    if(test_group_setup == NULL || num_group_setup == 0) {
        /* nothing to do */
        return 0;
    }
    
    for(i = 0; i < num_group_setup; i++) {
        if(test_group_setup[i].teardown == NULL) {
            continue;
        }
        retval = test_group_setup[i].teardown(state);
        if(retval) {
            return retval;
        }
    }
    
    return 0;
    
}

/* 
 * Default unit test and setup functions to avoid 
 * undefined errors when UnitTest macros are not used
 */

UnitTest(default_test) {
    assert_true(1);
}

BeforeAll(default_setup) {
    return 0;
}


__attribute__((weak))
int main (int argc, char ** argv)
{

    discover_tests();
    
    return 
        _cmocka_run_group_tests(argv[0], tests, num_tests, _run_group_setup, _run_group_teardown);

}
