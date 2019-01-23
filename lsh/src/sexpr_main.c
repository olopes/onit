#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "sexpr.h"
#include "sobj.h"
#include "svisitor.h"

int
sexpr_main(int argc, char ** argv) {
    struct sexpr * lcons;
    struct sexpr * ccons;
    struct sexpr * pcons;
    struct sobj * lobj;
    struct sobj * sobj;
    struct sobj * nobj;
    struct sobj * aobj;
    struct sobj * cobj;
    struct sobj * pobj;
    wchar_t * sstring = L"Hellö Wørld!";
    wchar_t * ssym = L"thingy";
    wchar_t * sadd = L"add";
    
    /* this is needed for correct output, otherwise a '?' is displayed */
    setlocale(LC_ALL, "");
    
    /* manually build something like this */
    /* (thingy . ((add . "Hellö Wørld!") . NIL)) */
    
    nobj = sobj_from_symbol(ssym, wcslen(ssym));
    sobj = sobj_from_string(sstring, wcslen(sstring));
    aobj = sobj_from_symbol(sadd, wcslen(sadd));
    
    ccons = sexpr_cons(aobj, sobj);
    cobj = sobj_from_sexpr(ccons);
    
    pcons = sexpr_cons(cobj, NIL);
    pobj = sobj_from_sexpr(pcons);
    
    lcons = sexpr_cons(nobj, pobj);
    lobj = sobj_from_sexpr(lcons);
    
    wprintf(L"\nThis is dump_r lobj:\n");
    dump_sobj_r(lobj, stdout);
    
    wprintf(L"\nThis is nobj:\n");
    dump_sobj(nobj, stdout);
    
    wprintf(L"\nThis is sobj:\n");
    dump_sobj(sobj, stdout);
    
    wprintf(L"\nThis is aobj:\n");
    dump_sobj(aobj, stdout);
    
    wprintf(L"\nThis is cobj:\n");
    dump_sobj(cobj, stdout);
    
    wprintf(L"\nThis is pobj:\n");
    dump_sobj(pobj, stdout);
    
    wprintf(L"\nThis is lobj:\n");
    dump_sobj(lobj, stdout);
   
    wprintf(L"\nDone!\n");
    
    /* release resources */
    sexpr_free(ccons);
    sexpr_free(pcons);
    sexpr_free(lcons);
    
    sobj_free(cobj);
    sobj_free(pobj);
    sobj_free(lobj);
    sobj_free(sobj);
    sobj_free(nobj);
    sobj_free(aobj);
    
    return 0;
}
