#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "shash.h"

/* 
 * note to self: I'm using a power-of-two sized table with quadric probing resolution:
 * https://en.wikipedia.org/wiki/Quadratic_probing
 * http://www.chilton-computing.org.uk/acl/literature/reports/p012.htm
 */


#define hashcode(a) sexpr_hashcode(a); 
#define compare(a, b) sexpr_compare(a, b)
#define DEFAULT_HASHTABLE_SIZE 16
static int resize_table_if_necessary(struct shash_table * hashtable);
static inline int should_resize_table(struct shash_table * hashtable);
static void rehash(struct shash_table * new_table, struct shash_table * old_table);
static void insert(struct shash_table * hashtable, struct sexpression * key, void * value);
static int get_index(struct shash_table * hashtable, struct sexpression * key, size_t * indexptr);


int shash_insert(struct shash_table * hashtable, struct sexpression * key, void * value) {
    
    if(hashtable == NULL || !sexpr_is_symbol(key)) {
        return 1;
    }
    
    if(resize_table_if_necessary(hashtable)) {
        /* resize failed */
        return 1;
    }
    
    insert(hashtable, key, value);
    
    return 0;
}

static int resize_table_if_necessary(struct shash_table * hashtable) {
    struct shash_table new_table;
    
    if(!should_resize_table(hashtable)) {
        return 0;
    }
    
    if(hashtable->size == 0) {
        new_table.size = DEFAULT_HASHTABLE_SIZE;
    } else {
        new_table.size = hashtable->size * 2;
    }
    
    new_table.load = 0;
    
    new_table.table = (struct shash_entry *) malloc(sizeof(struct shash_entry) * new_table.size);
    if(new_table.table == NULL) {
        return 1;
    }
    memset(new_table.table, 0, sizeof(struct shash_entry) * new_table.size);
    
    if(hashtable->table != NULL) {
        rehash(&new_table, hashtable);
        free(hashtable->table);
    }
    
    *hashtable = new_table;

    return 0;
}

static inline int should_resize_table(struct shash_table * hashtable) {
    return hashtable->load >= 3*hashtable->size/4;
}

static void rehash(struct shash_table * new_table, struct shash_table * old_table) {
    size_t i;
    struct shash_entry * table;
    
    for(i = 0, table = old_table->table; i < old_table->size; i++, table++) {
        if(table->key != NULL) {
            insert(new_table, table->key, table->value);
        }
    }
}


static void insert(struct shash_table * hashtable, struct sexpression * key, void * value) {
    unsigned long hash;
    size_t i;
    size_t j;
    
    hash = hashcode(key);
    for(j = 0; j < hashtable->size; j++) {
        i = (hash + j * j) %  hashtable->size;
        if(hashtable->table[i].key == NULL) {
            hashtable->table[i].key = key;
            hashtable->table[i].value = value;
            hashtable->load++;
            break;
        }
    }
    
}

void * shash_delete(struct shash_table * hashtable, struct sexpression * key) {
    size_t index;
    struct sexpression * value;
    
    if(!get_index(hashtable, key, &index)) {
        return NULL;
    }
    
    value = hashtable->table[index].value;
    hashtable->table[index].key = NULL;
    hashtable->table[index].value = NULL;
    hashtable->load--;
    
    return value;
}

static int get_index(struct shash_table * hashtable, struct sexpression * key, size_t * index) {
    unsigned long hash;
    size_t j;
    size_t i;
    
    if(hashtable == NULL || hashtable->load == 0 || !sexpr_is_symbol(key)) {
        return 0;
    }
    
    hash = hashcode(key);
    for(j = 0; j < hashtable->size; j++) {
        i = (hash + j * j) %  hashtable->size;
        if(hashtable->table[i].key != NULL && compare(key, hashtable->table[i].key) == 0) {
            /* found it! */
            *index = i;
            return 1;
        }
    }
    
    return 0;
}

int shash_has_key(struct shash_table * hashtable, struct sexpression * key ) {
    size_t index;
    return get_index(hashtable, key, &index);
}

void * shash_search(struct shash_table * hashtable, struct sexpression * key ) {
    struct shash_entry * entry;
    
    entry = shash_get_entry(hashtable, key);
    
    if(entry == NULL) {
        return NULL;
    }
    
    return entry->value;
}

struct shash_entry *
shash_get_entry(struct shash_table * hashtable, struct sexpression * key ) {
    size_t index;
    
    if(!get_index(hashtable, key, &index)) {
        return NULL;
    }
    
    return &hashtable->table[index];
}

void shash_visit(struct shash_table * hashtable, void * param, void (*callback)(void * param, struct sexpression * key, void * value)) {
    size_t i;
    struct shash_entry * table;
    
    for(i = 0, table = hashtable->table; i < hashtable->size; i++, table++) {
        if(table->key != NULL) {
            (*callback)(param, table->key, table->value);
        }
    }
}


void shash_free(struct shash_table * hashtable) {
    if(hashtable == NULL) {
        return;
    }
    
    if(hashtable->table != NULL) {
        free(hashtable->table);
    }
    
    memset(hashtable, 0, sizeof(struct shash_table));
}

