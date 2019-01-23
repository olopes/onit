#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#include "ostr.h"

int 
ostr_main(int argc, char ** argv) {
    struct ostr * str;
    wint_t c;
    
    setlocale(LC_ALL, "");
    
    str = ostr_new(NULL);
    
    fputws(L"Say something nice:\n", stdout);
    
    while(1) {
        c = getwchar();
        if(c == WEOF) {
            break;
        }
        
        str = ostr_append(str, c);
    }
    
    fputws(L"\nSo, you said this to me:\n", stdout);
    
    ostr_puts(str);
    
    wprintf(L"\nBy the way, the length of your string is %d.\nBye bye!\n", ostr_length(str));
    
    ostr_destroy(str);
    
    return 0;
}
