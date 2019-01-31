
/* include this file to override fgetwc and ungetwc in your tests */

#include <stdio.h>
#include <wchar.h>

/* mocks and stubs */
struct mock_file {
    wchar_t * data;
    int fget_calls;
    int unget_calls;
    size_t size;
    size_t position;
    int unget_flag;
} MOCK_FILE;

void mock_io(wchar_t * chars, size_t size) {
    MOCK_FILE.data = chars;
    MOCK_FILE.fget_calls = 0;
    MOCK_FILE.unget_calls = 0;
    MOCK_FILE.position = 0;
    MOCK_FILE.size = size;
    MOCK_FILE.unget_flag = 0;
}

void verify_fgetwc(int calls) {
    assert_int_equal(MOCK_FILE.fget_calls, calls);
}

void verify_ungetwc(int calls) {
    assert_int_equal(MOCK_FILE.unget_calls, calls);
}

wint_t __wrap_fgetwc(FILE * stream)
{
    wint_t chr;
    
    MOCK_FILE.fget_calls++;
    MOCK_FILE.unget_flag = 0;
    
    if(MOCK_FILE.position < MOCK_FILE.size) {
        chr = *MOCK_FILE.data;
        MOCK_FILE.data++;
        MOCK_FILE.position++;
    } else {
        chr = WEOF;
    }
    
    return chr;
}

wint_t __wrap_ungetwc(wint_t chr, FILE * stream)
{
    MOCK_FILE.unget_calls++;
    
    if(MOCK_FILE.unget_flag) {
        fail_msg("ungetwc() called twice without fgetwc() in the middle");
    } else if(MOCK_FILE.position > 0) {
        MOCK_FILE.unget_flag = 1;
        MOCK_FILE.position--;
        MOCK_FILE.data--;
        
        assert_int_equal(chr, *MOCK_FILE.data);
    } else {
        fail_msg("Tried to call ungetwc at the begining of the stream.");
    }
        
    return chr;
}

