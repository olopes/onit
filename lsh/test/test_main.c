#include "test_definitions.h"
#include <stdio.h>


/* injected automatically by the linker script */
extern struct LshUnitTest __start_lshtest;
extern struct LshUnitTest __stop_lshtest;
extern struct GroupSetupTeardown __start_lshsetup;
extern struct GroupSetupTeardown __stop_lshsetup;

/* placeholders for the linker script */
UnitTest(default_test, .ignore=1) { assert_true(1); }
BeforeAll(default_setup) { return 0; }


static void discover_tests(void);
static void release_tests(void);
static void discover_group_setup_teardown(void);
static void discover_unit_tests(void);
static size_t 
count_number_of_unit_tests(struct LshUnitTest * const start, struct LshUnitTest * const stop);
static struct CMUnitTest * alloc_unit_tests_array(size_t num_tests);
static void 
copy_unit_tests(struct LshUnitTest * const start, struct LshUnitTest * const stop, struct CMUnitTest * tests);
static int _run_group_setup(void ** state);
static int _run_group_teardown(void ** state);
static void ignored_test (void ** state);

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

    num_group_setup = 0;
    for(ptr = test_group_setup; ptr < &__stop_lshsetup; ptr++) {
        num_group_setup++;
    }
    
}


static void discover_unit_tests(void) {

    num_tests = count_number_of_unit_tests(&__start_lshtest, &__stop_lshtest);
    
    tests = alloc_unit_tests_array(num_tests);
    
    copy_unit_tests(&__start_lshtest, &__stop_lshtest, tests);
}

static size_t 
count_number_of_unit_tests(struct LshUnitTest * const start, struct LshUnitTest * const stop) {
    struct LshUnitTest * ptr;
    size_t num_tests;
    
    num_tests = 0;
    for(ptr = start; ptr < stop; ptr++) {
        if(ptr->test_func == default_test) {
            continue;
        }
        
        if(ptr->repeat > 1 && !ptr->ignore) {
            num_tests += ptr->repeat;
        } else {
            num_tests++;
        }
    }
    return num_tests;
}


static struct CMUnitTest * alloc_unit_tests_array(size_t num_tests) {
    struct CMUnitTest * tests;
    
    tests = (struct CMUnitTest *) malloc(sizeof(struct CMUnitTest) * num_tests);
    
    if(tests == NULL) {
        fprintf(stderr, "ERROR: Error allocating test case array\n");
        fflush(stderr);
        fflush(stdout);
        abort();
    }
    
    return tests;
}


static void 
copy_unit_tests(struct LshUnitTest * const start, struct LshUnitTest * const stop, struct CMUnitTest * tests) {
    struct LshUnitTest * ptr;
    CMUnitTestFunction fun_ptr;
    size_t i;
    size_t repeat;
    
    for(i = 0, ptr = start; ptr < stop; ptr++) {
        if(ptr->test_func == default_test) {
            continue;
        }
        
        if(ptr->repeat > 1 && !ptr->ignore) {
            repeat = ptr->repeat;
        } else {
            repeat = 1;
        }
        
        fun_ptr = ptr->ignore ? ignored_test : ptr->test_func;
        
        do {
            tests[i] = (struct CMUnitTest) {
                .name = ptr->name,
                .test_func = fun_ptr,
                .setup_func = ptr->setup,
                .teardown_func = ptr->teardown,
                .initial_state = ptr->initial_state,
            };
            
            i++;
            repeat--;
        } while(repeat > 0);
        
    }
}

static void ignored_test (void ** state) {
    skip();
}


static void release_tests(void)
{
    if(tests == NULL) {
        return;
    }
    
    free(tests);
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

__attribute__((weak))
int main (int argc, char ** argv)
{

    discover_tests();
    
    int num_tests_failed =
        _cmocka_run_group_tests(argv[0], tests, num_tests, _run_group_setup, _run_group_teardown);

    release_tests();
    
    return num_tests_failed;
}
