#ifndef _SHASH_H_
#define _SHASH_H_

#include "sexpr.h"

struct shash_entry {
    struct sexpression * key;
    void * value;
};

struct shash_table {
    size_t size;
    size_t load;
    struct shash_entry * table;
};

extern int 
shash_insert(struct shash_table * hashtable, struct sexpression * key, void * value);

extern void * 
shash_delete(struct shash_table * hashtable, struct sexpression * key);

extern int 
shash_has_key(struct shash_table * hashtable, struct sexpression * key );

extern void * 
shash_search(struct shash_table * hashtable, struct sexpression * key );

extern void 
shash_visit(struct shash_table * hashtable, void * param, void (*callback)(void * param, struct sexpression * key, void * value));

extern void 
shash_free(struct shash_table * hashtable);

#endif
