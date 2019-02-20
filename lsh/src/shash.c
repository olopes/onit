#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "shash.h"


#define hashcode(a) svalue_hashcode(a); 
#define compare(a, b) svalue_compare(a, b)
#define DEFAULT_HASHTABLE_SIZE 16
static int resize_table(struct shash_table * hashtable);


struct shash_table * shash_create() {
    struct shash_table * hashtable;
    
    hashtable = (struct shash_table *) malloc(sizeof(struct shash_table));
    if(hashtable == NULL) {
        return NULL;
    }
    memset(hashtable, 0, sizeof(struct shash_table));
    
    hashtable->size = DEFAULT_HASHTABLE_SIZE;
    hashtable->table = (struct shash_entry **) malloc(sizeof(struct shash_entry*) * DEFAULT_HASHTABLE_SIZE);
    if(hashtable->table == NULL) {
        free(hashtable);
        return NULL;
    }
    memset(hashtable->table, 0, sizeof(struct shash_entry*) * DEFAULT_HASHTABLE_SIZE);
    
    return hashtable;
}

int shash_insert(struct shash_table * hashtable, struct svalue * key, struct sexpression * value) {
    
    
    return 0;
}

struct sexpression * shash_delete(struct shash_table * hashtable, struct svalue * key);

int hash_has_key(struct shash_table * hashtable, struct svalue * key );

struct sexpression * hash_search(struct shash_table * hashtable, struct svalue * key );

void hash_visit(struct shash_table * hashtable, void (*callback)(struct svalue * key, struct sexpression * value));



