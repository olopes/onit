#include <stdio.h>
#include <wchar.h>
#include <wctype.h>

/*
fputwc
fgetwc
*/

wint_t __real_fputwc(wchar_t wc, FILE *stream);
__attribute__((weak))
wint_t WEAK_FOR_UNIT_TEST
__wrap_fputwc(wchar_t wc, FILE *stream) {

    return __real_fputwc(wc, stream);
}

wint_t __real_fgetwc(FILE *stream);
__attribute__((weak))
wint_t WEAK_FOR_UNIT_TEST
__wrap_fgetwc(FILE *stream) {

    return __real_fgetwc(stream);
}

wint_t WEAK_FOR_UNIT_TEST
__wrap_getwc(FILE *stream) {

    return __wrap_fgetwc(stream);
}

wint_t __real_ungetwc(wint_t c, FILE *stream);
__attribute__((weak))
wint_t WEAK_FOR_UNIT_TEST
__wrap_ungetwc(wint_t c, FILE *stream) {

    return __real_ungetwc(c, stream);
}


