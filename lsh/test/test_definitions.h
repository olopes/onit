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
    const char *name;
    CMUnitTestFunction test_func;
    CMFixtureFunction setup;
    CMFixtureFunction teardown;
    void *initial_state;
    size_t repeat;
    char ignore;
    char _filler;
} __attribute__((aligned(64)));


struct GroupSetupTeardown {
    CMFixtureFunction setup;
    CMFixtureFunction teardown;
} __attribute__((aligned(64)));

#define sexpr_create_csymbol(x) sexpr_create_symbol((x), wcslen(x))
#define sexpr_create_cstring(x) sexpr_create_string((x), wcslen(x))

#define LSH_EXPAND(x) x

#define __UnitTestSetupTeadown(test_name, ...) \
static void test_name (void ** state); \
struct LshUnitTest __attribute__ ((section ("lshtest"))) _ ## test_name = { \
    .name = #test_name, \
    .test_func = test_name, \
    __VA_ARGS__ \
}; \
static void test_name (void ** state)

#define UnitTest(...) LSH_EXPAND(__UnitTestSetupTeadown(__VA_ARGS__, ._filler=0))

#if defined(__attribute__) && defined(__TINYC__)
#undef __attribute__
#endif

#define BeforeAll(setup_name) \
static int setup_name (void ** state); \
struct GroupSetupTeardown __attribute__ ((section ("lshsetup"))) _ ## setup_name = { \
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
