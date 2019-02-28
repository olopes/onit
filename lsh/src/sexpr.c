#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "sexpr.h"
#include "sexpr_stack.h"

/**
 * Constructs a pair (A . B)
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_cons(struct sexpression * car, struct sexpression * cdr) {
    struct sexpression * sexpr;
    
    sexpr = (struct sexpression *) malloc(sizeof(struct sexpression));
    if(sexpr == NULL) {
        return NULL;
    }
    
    memset(sexpr, 0, sizeof(struct sexpression));
    sexpr->type = ST_CONS;
    sexpr->data = car;
    sexpr->cdr = cdr;
    
    if(sexpr_is_cons(cdr)) {
        sexpr->len = cdr->len + 1;
    } else {
        sexpr->len = 0;
    }
    
    return sexpr;
}

/**
 * Create a S-Expression value
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_create_value(wchar_t * cstr, size_t len) {
    struct sexpression * sexpr;
    
    if(cstr == NULL) {
        return NULL;
    }
    
    sexpr = sexpr_cons(NULL, NULL);
    if(sexpr == NULL) {
        return NULL;
    }
    
    sexpr->type = ST_VALUE;
    sexpr->len = len;
    sexpr->data = malloc(sizeof(wchar_t)*(len+1));
    if(sexpr->data == NULL) {
        sexpr_free(sexpr);
        return NULL;
    }
    memcpy(sexpr->data, cstr, sizeof(wchar_t)*len);
    ((wchar_t *) sexpr->data)[len] = L'\0';
    
    return sexpr;
}


void WEAK_FOR_UNIT_TEST
sexpr_free(struct sexpression * sexpr) {
    if(sexpr == NULL) {
        return;
    }
    
    if(sexpr_is_cons(sexpr)) 
    {
        sexpr_free(sexpr_car(sexpr));
        sexpr_free(sexpr_cdr(sexpr));
    }
    else if(sexpr_is_value(sexpr)) 
    {
        free(sexpr->data);
    }
        
    free(sexpr);
}

void WEAK_FOR_UNIT_TEST
sexpr_free_object(struct sexpression * sexpr) {
    if(sexpr == NULL) {
        return;
    }
    
    if(sexpr_is_value(sexpr)) {
        free(sexpr->data);
    }
    free(sexpr);
}

/**
 * Get the CAR part
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_car(struct sexpression *sexpr) {
    /* how to raise error? */
    return sexpr_type(sexpr) == ST_CONS ? (struct sexpression *) sexpr->data : NULL;
}

/**
 * Get the CDR part
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_cdr(struct sexpression *sexpr) {
    /* how to raise error? */
    return sexpr_type(sexpr) == ST_CONS ? (struct sexpression *) sexpr->cdr : NULL;
}

/**
 * Get S-Expression value 
 */
struct svalue * WEAK_FOR_UNIT_TEST
sexpr_value(struct sexpression *sexpr) {
    /* FIXME I'm assuming things here... */
    return sexpr_type(sexpr) == ST_VALUE ? (struct svalue *) sexpr : NULL;
}

/**
 * Get the S-Expression type
 */
unsigned char
sexpr_type(struct sexpression * sexpr) {
    return sexpr == NULL ? ST_NIL : sexpr->type ;
}

/**
 * Return TRUE if the given object is NIL
 */
int
sexpr_is_nil(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_NIL;
}

/**
 * Return TRUE if the given object is a CONS pair
 */
int
sexpr_is_cons(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_CONS;
}

/**
 * Return TRUE if the given object is a string
 */
int
sexpr_is_value(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_VALUE;
}


/**
 * Return TRUE if both S-Expressions are equal
 */
int sexpr_equal(struct sexpression * a, struct sexpression * b) {
    int are_equal;
    
    /* same object ? */
    if(a == b) {
        are_equal = 1;
    } else if(a == NULL || b == NULL) {
        are_equal = 0;
    } else if (sexpr_type(a) != sexpr_type(b)) {
        are_equal = 0;
    } else if (sexpr_is_cons(a)) {
        are_equal = (sexpr_equal(sexpr_car(a), sexpr_car(b)) && sexpr_equal(sexpr_cdr(a), sexpr_cdr(b)));
    } else {
        are_equal = wcsncmp((wchar_t *) a->data, (wchar_t *) b->data, a->len) == 0;
    }
    return are_equal;
}

/**
 * Reverse a S-Expression list
 */
struct sexpression * 
sexpr_reverse(struct sexpression * sexpr) {
    struct sexpression * car;
    struct sexpression * cdr;
    struct sexpression * iter;
    struct sexpression * reversed;
    size_t length;
    
    if(!sexpr_is_cons(sexpr)) {
        return sexpr;
    }
    
    car = sexpr_car(sexpr);
    cdr = sexpr_cdr(sexpr);
    
    /* handle a "list" with size 1*/
    if(sexpr_is_nil(cdr)) {
        sexpr->len = 1;
        return sexpr;
    }
    
    /* handle a pair */
    if(!sexpr_is_cons(cdr)) {
        sexpr->data = cdr;
        sexpr->cdr = car;
        return sexpr;
    }
    
    /* handle a "list" */
    
    length = 0;
    reversed = NULL;
    iter = sexpr;
    while(iter) {
        length++;
        iter->len = length;
        cdr = sexpr_cdr(iter);
        
        /* manipulate pointers */
        iter->cdr = reversed;
        reversed = iter;
        iter = cdr;
    }
    
    return reversed;
}



void
sexpr_mark_reachable(struct sexpression * object, unsigned char visit_mark) {
    struct sexpression * sexpr = object;
    
    while(sexpr != NULL) {
        sexpr->visit_mark = visit_mark;
        sexpr_mark_reachable(sexpr_car(sexpr), visit_mark);
        sexpr = sexpr_cdr(sexpr);
    }
    
}

int
sexpr_marked(struct sexpression * sexpr, unsigned char visit_mark) {
    return sexpr != NULL && sexpr->visit_mark == visit_mark;
}
