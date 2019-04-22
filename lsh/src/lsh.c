#include <wchar.h>
#include <locale.h>
#include "sparser.h"
#include "stdio_sparser_adapter.h"
#include "svisitor.h"

int
lsh_main(int argc, char ** argv) {
    struct sexpression * object;
    struct sparser_stream * stream;
    int return_value;
    
    /* configure IO to handle wchar_t correctly */
    
    setlocale(LC_ALL, "");
    
    wprintf(L"S-Expression Parser POC\n> ");
    fflush ( stdout );
    stream = create_sparser_stream( FILE_DESCRIPTOR_ADAPTER, stdin);
    
    while((return_value=sparse(stream, &object)) != SPARSE_EOF) {
        if(return_value == SPARSE_OK) {
            wprintf(L"%ls", L"New object read from stdin\n");
            dump_sexpr_r(object, stdout);
            sexpr_free(object);
        } else {
            wprintf(L"Bad input: %d\n", return_value);
        }
        wprintf(L"\n> ", return_value);
        fflush ( stdout );
    }
    
    wprintf(L"\nEOF found\nBye bye!\n");
    
    return 0;
}
