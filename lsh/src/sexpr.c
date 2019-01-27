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
    if(sexpr) 
    {
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
}

void WEAK_FOR_UNIT_TEST
sexpr_free_pair(struct sexpression * sexpr) {
    if(sexpr_is_cons(sexpr)) {
        free(sexpr);
    }
}

/**
 * Get the CAR part
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_car(struct sexpression *sexpr) {
    /* how to raise error? */
    return sexpr && sexpr->type == ST_CONS ? (struct sexpression *) sexpr->data : NULL;
}

/**
 * Get the CDR part
 */
struct sexpression * WEAK_FOR_UNIT_TEST
sexpr_cdr(struct sexpression *sexpr) {
    /* how to raise error? */
    return sexpr && sexpr->type == ST_CONS ? (struct sexpression *) sexpr->cdr : NULL;
}

/**
 * Get S-Expression value 
 */
struct svalue * WEAK_FOR_UNIT_TEST
sexpr_value(struct sexpression *sexpr) {
    return sexpr && sexpr->type == ST_VALUE ? (struct svalue *) sexpr->data : NULL;
}

/**
 * Get the S-Expression type
 */
enum stype
sexpr_type(struct sexpression * sexpr) {
    return sexpr ? sexpr->type : ST_NIL;
}

/**
 * Return TRUE if the given object is NIL
 */
int
sexpr_is_nil(struct sexpression * sexpr) {
    return sexpr == NULL || sexpr->type == ST_NIL;
}

/**
 * Return TRUE if the given object is a CONS pair
 */
int
sexpr_is_cons(struct sexpression * sexpr) {
    return sexpr && sexpr->type == ST_CONS;
}

/**
 * Return TRUE if the given object is a string
 */
int
sexpr_is_value(struct sexpression * sexpr) {
    return sexpr && sexpr->type == ST_VALUE;
}

