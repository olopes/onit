#include <errno.h>
#include <stdlib.h>
#include "sobj.h"
#include "sexpr.h"

static struct sobj NIL_OBJ = { 0, NULL, T_NIL};
struct sobj * NIL = &NIL_OBJ;

/**
 * Create a S-Expression object from a string
 */
struct sobj * WEAK_FOR_UNIT_TEST
sobj_from_string(wchar_t * value, size_t len) {
    struct sobj * obj;
    
    obj = (struct sobj *) malloc(sizeof(struct sobj));
    if(obj == NULL) {
        return NULL;
    }
    
    obj->len=len;
    obj->data = value;
    obj->type = T_STRING;
    
    return obj;
}


/**
 * Create a S-Expression object from a symbol
 */
struct sobj * WEAK_FOR_UNIT_TEST
sobj_from_symbol(wchar_t * value, size_t len) {
    struct sobj * obj;
    
    obj = sobj_from_string(value, len);
    if(obj == NULL) {
        return NULL;
    }
    
    obj->type = T_SYMBOL;
    
    return obj;
}

/**
 * Create a S-Expression object from a S-Expression
 */
struct sobj * WEAK_FOR_UNIT_TEST
sobj_from_cons(struct sexpr * value) {
    struct sobj * obj;
    
    obj = (struct sobj *) malloc(sizeof(struct sobj));
    if(obj == NULL) {
        return NULL;
    }
    
    obj->len = -1;
    obj->data = value;
    obj->type = T_CONS;
    
    return obj;
}

/**
 * Release a S-Expression Object created by sobj_from_*
 */
void WEAK_FOR_UNIT_TEST
sobj_free(struct sobj * obj) {
    if(sobj_is_nil(obj)) {
        return;
    }
    free(obj);
}

/**
 * Fetch the S-Expression object value as a string
 */
struct sexpr_str * WEAK_FOR_UNIT_TEST
sobj_to_string(struct sobj * obj) {
    if(sobj_is_string(obj)) {
        return (struct sexpr_str *) obj;
    }
    return NULL;
}

/**
 * Fetch the S-Expression object value as a symbol
 */
struct sexpr_str * WEAK_FOR_UNIT_TEST
sobj_to_symbol(struct sobj * obj) {
    if(sobj_is_symbol(obj)) {
        return (struct sexpr_str *) obj;
    }
    return NULL;
}

/**
 * Fetch the S-Expression object value as a S-Expression
 */
struct sexpr * WEAK_FOR_UNIT_TEST
sobj_to_cons(struct sobj * obj) {
    if(sobj_is_cons(obj)) {
        return (struct sexpr *) obj->data;
    }
    return NULL;
}

/**
 * Get the object sobj_get_type
 */
enum stype WEAK_FOR_UNIT_TEST
sobj_get_type(struct sobj * obj) {
    return obj->type;
}

/**
 * Return TRUE if the given object is NIL
 */
int WEAK_FOR_UNIT_TEST
sobj_is_nil(struct sobj * obj) {
    return obj && obj->type==T_NIL;
}

/**
 * Return TRUE if the given object is a CONS pair
 */
int WEAK_FOR_UNIT_TEST
sobj_is_cons(struct sobj * obj) {
    return obj && obj->type==T_CONS;
}

/**
 * Return TRUE if the given object is a string
 */
int WEAK_FOR_UNIT_TEST
sobj_is_string(struct sobj * obj) {
    return obj && obj->type==T_STRING;
}

/**
 * Return TRUE if the given object is a symbol
 */
int WEAK_FOR_UNIT_TEST
sobj_is_symbol(struct sobj * obj) {
    return obj && obj->type==T_SYMBOL;
}

