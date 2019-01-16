#ifndef __STYPE__H__
#define __STYPE__H__

#include <wchar.h>

struct sstr {
    size_t len;
    wchar_t *data;
};

enum stype {T_NIL, T_CONS, T_STRING, T_SYMBOL};
    
struct sobj { 
    size_t len;
    void *data;
    enum stype type;
};

struct sexpr {
    struct sobj * car;
    struct sobj * cdr;
};

#endif /* __STYPE__H__ */
