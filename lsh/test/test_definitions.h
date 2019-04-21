#ifndef UNIT_TESTING
#define UNIT_TESTING

#define WEAK_FOR_UNIT_TEST __attribute__((weak))

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <wctype.h>
#include <wchar.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#include <cmocka.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/* remove macro defs to use real function declaration */
#ifdef fgetwc
#undef fgetwc
#endif

#ifdef getwc
#undef getwc
#endif

#ifdef fputwc
#undef fputwc
#endif

#ifdef ungetwc
#undef ungetwc
#endif

/* Useful struct for testing with multiple parameters */
struct sparser_test_params {
    wchar_t * stream;
    wchar_t * expected;
    int return_value;
    int expected_fgetc_calls;
};

struct LshUnitTest {
    struct CMUnitTest test;
    int filler;
} __attribute__((aligned(64)));


struct GroupSetupTeardown {
    CMFixtureFunction setup;
    CMFixtureFunction teardown;
} __attribute__((aligned(64)));

#define sexpr_create_cstr(x) sexpr_create_value((x), wcslen(x))

#define UnitTestSetupTeadown(test_name, setup, teadown) \
static void test_name (void ** state); \
struct LshUnitTest __attribute__((section ("lshtest"))) _ ## test_name = { \
    .test = { \
        .name = #test_name, \
        .test_func = test_name, \
        .setup_func = setup, \
        .teardown_func = teadown, \
        .initial_state = NULL \
    }, \
    .filler = 0 \
}; \
static void test_name (void ** state)

#define UnitTest(test_name) UnitTestSetupTeadown(test_name, NULL, NULL)

#define UnitTestSetup(test_name, setup) UnitTestSetupTeadown(test_name, setup, NULL)

#define UnitTestTeardown(test_name, teardown ) UnitTestSetupTeadown(test_name, NULL, teardown)

#define BeforeAll(setup_name) \
static int setup_name (void ** state); \
struct GroupSetupTeardown __attribute__((section ("lshsetup"))) _ ## setup_name = { \
    .setup = setup_name, \
    .teardown = NULL \
}; \
static int setup_name (void ** state)

#define AfterAll(teardown_name) \
static int teardown_name (void ** state); \
struct GroupSetupTeardown __attribute__((section ("lshsetup"))) _ ## teardown_name = { \
    .setup = NULL, \
    .teardown = teardown_name \
}; \
static int teardown_name (void ** state)

#endif /* UNIT_TESTING */
