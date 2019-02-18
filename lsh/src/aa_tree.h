#ifndef _AA_TREE_H_
#define _AA_TREE_H_

#include "sexpr.h"

struct aa_node;

struct aa_tree {
    struct aa_node * root;
};

struct aa_node {
    struct svalue * key;
    struct sexpression * value;
    size_t level;
    struct aa_node * left;
    struct aa_node * right;
};

int aa_insert(struct aa_tree * tree, struct svalue * key, struct sexpression * value);

struct sexpression * aa_delete(struct aa_tree * tree, struct svalue * key);

int aa_has_key(struct aa_tree * tree, struct svalue * key );

struct sexpression * aa_search(struct aa_tree * tree, struct svalue * key );

void aa_visit(struct aa_tree * tree, void (*callback)(struct svalue * key, struct sexpression * value));

#endif
