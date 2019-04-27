#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "sexpr.h"
#include "svisitor.h"

int
sexpr_main(int argc, char ** argv) {
    struct sexpression * lobj;
    struct sexpression * sobj;
    struct sexpression * nobj;
    struct sexpression * aobj;
    struct sexpression * cobj;
    struct sexpression * pobj;
    wchar_t * sstring = L"Hellö Wørld!";
    wchar_t * ssym = L"thingy";
    wchar_t * sadd = L"add";
    
    /* this is needed for correct output, otherwise a '?' is displayed */
    setlocale(LC_ALL, "");
    
    /* manually build something like this */
    /* (thingy . ((add . "Hellö Wørld!") . NIL)) */
    
    nobj = sexpr_create_string(ssym, wcslen(ssym));
    sobj = sexpr_create_symbol(sstring, wcslen(sstring));
    aobj = sexpr_create_symbol(sadd, wcslen(sadd));
    
    cobj = sexpr_cons(aobj, sobj);
    
    pobj = sexpr_cons(cobj, NULL);
    
    lobj = sexpr_cons(nobj, pobj);
    
    wprintf(L"\nThis is dump_r lobj:\n");
    dump_sexpr_r(lobj, stdout);
    
    wprintf(L"\nThis is nobj:\n");
    dump_sexpr(nobj, stdout);
    
    wprintf(L"\nThis is sobj:\n");
    dump_sexpr(sobj, stdout);
    
    wprintf(L"\nThis is aobj:\n");
    dump_sexpr(aobj, stdout);
    
    wprintf(L"\nThis is cobj:\n");
    dump_sexpr(cobj, stdout);
    
    wprintf(L"\nThis is pobj:\n");
    dump_sexpr(pobj, stdout);
    
    wprintf(L"\nThis is lobj:\n");
    dump_sexpr(lobj, stdout);
   
    wprintf(L"\nDone!\n");
    
    /* release resources */
    sexpr_free(lobj);
    
    return 0;
}
