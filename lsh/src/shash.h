#ifndef _SHASH_H_
#define _SHASH_H_

#include "sexpr.h"

struct shash_entry {
    struct svalue * key;
    struct sexpression * value;
};

struct shash_table {
    size_t size;
    size_t load;
    struct shash_entry ** table;
};

extern struct shash_table * shash_create();

extern int shash_insert(struct shash_table * hashtable, struct svalue * key, struct sexpression * value);

extern struct sexpression * shash_delete(struct shash_table * hashtable, struct svalue * key);

extern int hash_has_key(struct shash_table * hashtable, struct svalue * key );

extern struct sexpression * hash_search(struct shash_table * hashtable, struct svalue * key );

extern void hash_visit(struct shash_table * hashtable, void (*callback)(struct svalue * key, struct sexpression * value));

#endif
