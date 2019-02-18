#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "aa_tree.h"


static struct aa_node * insert(struct aa_node * subtree, struct svalue * key, struct sexpression * value);
static struct aa_node * skew(struct aa_node * node);
static struct aa_node * split(struct aa_node * node);
static struct aa_node * delete(struct aa_node * subtree, struct svalue * key);
static struct aa_node * successor(struct aa_node * node);
static struct aa_node * predecessor(struct aa_node * node);
static struct aa_node * decrease_level(struct aa_node * node);
static struct aa_node * left(struct aa_node * node);
static struct aa_node * right(struct aa_node * node);
static size_t level(struct aa_node * node);
static size_t min(size_t a, size_t b);
static int leaf(struct aa_node * node);
static int compare(struct svalue * a, struct svalue * b);
static void visit(struct aa_node * node, void (*callback)(struct svalue * key, struct sexpression * value));


int aa_insert(struct aa_tree * tree, struct svalue * key, struct sexpression * value) {
    struct aa_node * node;
    
    if(tree == NULL || key == NULL) {
        return 1;
    }
    
    node = insert(tree->root, key, value);
    if(node == NULL) {
        return 1;
    }
    
    /* do stuff */
    
    tree->root = node;
    return 0;
}


static struct aa_node * insert(struct aa_node * subtree, struct svalue * key, struct sexpression * value) {
    struct aa_node * node;
    int cmp;
    
    if (subtree == NULL) {
        node = (struct aa_node *) malloc(sizeof(struct aa_node));
        if(node == NULL) {
            return NULL;
        }
        memset(node, 0, sizeof(struct aa_node));
        node->key = key;
        node->value = value;
        node->level = 1;
        return node;
    }
    
    cmp = compare(key, subtree->key);
    if (cmp < 0) {
        node = insert(left(subtree), key, value);
        if(node == NULL) {
            return NULL;
        }
        subtree->left = node;
    } else if (cmp > 0) {
        node = insert(right(subtree), key, value);
        if(node == NULL) {
            return NULL;
        }
        subtree->right = node;
    }
    
    node = skew(subtree);
    node = split(node);
    return node;
}

static struct aa_node * skew(struct aa_node * node) {
    struct aa_node * left_node;
    
    if(node == NULL) {
        return NULL;
    }
    if(left(node) == NULL) {
        return node;
    }
    if(level(left(node)) == level(node)) {
        left_node = left(node);
        node->left = right(left_node);
        left_node->right = node;
        return left_node;
    }
    
    return node;
}

static struct aa_node * split(struct aa_node * node) {
    struct aa_node * right_node;
    
    if(node == NULL) {
        return NULL;
    }
    if(right(node) == NULL || right(right(node)) == NULL) {
        return node;
    }
    if(level(node) == level(right(right(node)))) {
        right_node = right(node);
        node->right = left(right_node);
        right_node->left = node;
        right_node->level++;
        return right_node;
    }
    
    return node;
}

static struct aa_node * left(struct aa_node * node) {
    return node == NULL ? NULL : node->left;
}

static struct aa_node * right(struct aa_node * node) {
    return node == NULL ? NULL : node->right;
}

static size_t level(struct aa_node * node) {
    return node == NULL ? 0 : node->level;
}

struct sexpression * aa_delete(struct aa_tree * tree, struct svalue * key) {
    struct aa_node * node;
    struct sexpression * value;
    
    if(tree == NULL || key == NULL) {
        return NULL;
    }
    
    value = aa_search(tree, key);
    
    node = delete(tree->root, key);
    if(node == NULL) {
        return NULL;
    }
    
    /* do stuff */
    
    tree->root = node;
    return value;
}


static struct aa_node * delete(struct aa_node * subtree, struct svalue * key) {
    struct aa_node * node;
    int cmp;
    
    if(subtree == NULL) {
        return NULL;
    }
    
    cmp = compare(key, subtree->key);
    if(cmp > 0) {
        subtree->right = delete(right(subtree), key);
    } else if (cmp < 0) {
        subtree->left = delete(left(subtree), key);
    } else {
        if(leaf(subtree)) {
            free(subtree);
            return NULL;
        } else if(left(subtree) == NULL) {
            /* swap right */
            node = successor(subtree);
            subtree->right = delete(right(subtree), node->key);
            subtree->key = node->key;
            subtree->value = node->value;
        } else {
            /* swap left */
            node = predecessor(subtree);
            subtree->left = delete(left(subtree), node->key);
            subtree->key = node->key;
            subtree->value = node->value;
        }
    }
    
    /* readjust level */
    node = decrease_level(subtree);
    node = skew(node);
    node->right = skew(right(node));
    if (right(node) != NULL) {
        node->right->right = skew(right(right(node)));
    }
    node = split(node);
    node->right = split(right(node));
    return node;
}

static struct aa_node * successor(struct aa_node * node) {
    if(node == NULL) {
        return NULL;
    }
    if(right(node) != NULL) {
        node = right(node);
        while(left(node) != NULL) {
            node = left(node);
        }
    }
    
    return node;
}

static struct aa_node * predecessor(struct aa_node * node) {
    if(node == NULL) {
        return NULL;
    }
    if(left(node)) {
        node = left(node);
        while(right(node) != NULL) {
            node = right(node);
        }
    }
    
    return node;
}

static struct aa_node * decrease_level(struct aa_node * node) {
    size_t should_be;
    
    /* check nulls ? */
    
    should_be = min(level(left(node)), level(right(node))) + 1;
    
    if (should_be < level(node)) {
        node->level = should_be;
        if (should_be < level(right(node))) {
            node->right->level = should_be;
        }
    }
    return node;
}

static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

static int leaf(struct aa_node * node) {
    return (left(node) == NULL && right(node) == NULL);
}

int aa_has_key(struct aa_tree * tree, struct svalue * key) {
    struct aa_node * node;
    int cmp;
    
    if(tree == NULL || key == NULL) {
        return 0;
    }
    
    node = tree->root;
    
    while(node) {
        cmp = compare(key, node->key);
        if (cmp == 0) {
            return 1;
        } else if(cmp < 0) {
            node = left(node);
        } else {
            node = right(node);
        }
    }
    
    return 0;
}

struct sexpression * aa_search(struct aa_tree * tree, struct svalue * key) {
    struct aa_node * node;
    int cmp;
    
    if(tree == NULL || key == NULL) {
        /* err = ENOENT; ?? */
        return NULL;
    }
    
    node = tree->root;
    
    while(node) {
        cmp = compare(key, node->key);
        if (cmp == 0) {
            return node->value;
        } else if(cmp < 0) {
            node = left(node);
        } else {
            node = right(node);
        }
    }
    
    return NULL;
}

static int compare(struct svalue * a, struct svalue * b) {
    if(a == b) {
        return 0;
    }
    if(a == NULL) {
        return -1;
    }
    if(b == NULL) {
        return 1;
    }
    return wcsncmp(a->data, b->data, a->len);
}

void aa_visit(struct aa_tree * tree, void (*callback)(struct svalue * key, struct sexpression * value)) {
    if(tree == NULL || callback == NULL) {
        return;
    }
    visit(tree->root, callback);
}

static void visit(struct aa_node * node, void (*callback)(struct svalue * key, struct sexpression * value)) {
    if(node == NULL) {
        return;
    }
    
    visit(left(node), callback);
    (*callback)(node->key, node->value);
    visit(right(node), callback);
}

