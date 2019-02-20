#ifndef __SVALUE__H__
#define __SVALUE__H__

#include <wchar.h>

struct svalue {
    size_t len;
    wchar_t *data;
};

extern unsigned long 
svalue_hashcode(struct svalue * key); 

extern int 
svalue_compare(struct svalue * a, struct svalue * b); 

#endif /* __SEXPR__H__ */
