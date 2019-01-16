#ifndef UNIT_TESTING
#define UNIT_TESTING

#define WEAK_FOR_UNIT_TEST __attribute__((weak))

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <wctype.h>
#include <wchar.h>
#include <cmocka.h>

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
struct parametrized_test_case {
    wchar_t * stream;
    wchar_t * expected;
    int retval;
    int calls;
};



#endif /* UNIT_TESTING */
