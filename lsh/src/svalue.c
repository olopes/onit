#include <string.h>
#include <wchar.h>
#include "svalue.h"


static inline size_t min(size_t a, size_t b); 

/* djb2 implementation */
unsigned long svalue_hashcode(struct svalue * key) {
    unsigned long hash;
    size_t i;
    if(key == NULL) {
        return 0;
    }
    
    hash = 5381;
    
    for(i = 0; i < key->len; i++) {
        /* hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        hash = hash * 33 ^ key->data[i];
    }
    return hash;
}

int svalue_compare(struct svalue * a, struct svalue * b) {
    size_t len;
    int cmp;
    if(a == b) {
        cmp = 0;
    } else if(a == NULL) {
        cmp = -1;
    } else if(b == NULL) {
        cmp = 1;
    } else {
        len = min(a->len, b->len);
        cmp = wmemcmp(a->data, b->data, len);
        if(cmp == 0) {
            cmp = a->len - b->len;
        }
    }
    
    return cmp;
}

static inline size_t min(size_t a, size_t b) {
    return a > b ? b : a;
}
