#ifndef __SEXPR__H__
#define __SEXPR__H__

#include <wchar.h>
#include "svalue.h"

/* object types */
#define ST_NIL   0
#define ST_CONS  1
#define ST_VALUE 2

/* referenced content type */
#define SC_PAIR 0
#define SC_STRING 1
#define SC_SYMBOL 2
#define SC_NUMBER 3
#define SC_PROCESS 4
#define SC_PRIMITIVE 5
#define SC_HASHTABLE 6
#define SC_VECTOR 7

struct sexpression { 
    size_t len;
    void *data;
    struct sexpression * cdr;
    unsigned char visit;
    unsigned char type;
    unsigned short content;
};

/**
 * Create a S-Expression pair (A . B)
 */
extern struct sexpression * 
sexpr_cons(struct sexpression * car, struct sexpression * cdr);

/**
 * Create a S-Expression value, usually a string, a symbol or a number
 */
extern struct sexpression * 
sexpr_create_value(wchar_t * cwstr, size_t length);

/**
 * Release a S-Expression
 */
extern void
sexpr_free(struct sexpression * sexpr);

/**
 * Release a single S-Expression object without releasing any referenced objects
 */
extern void
sexpr_free_object(struct sexpression * sexpr);

/**
 * Get the CAR part
 */
extern struct sexpression * 
sexpr_car(struct sexpression * sexpr);

/**
 * Get the CDR part
 */
extern struct sexpression * 
sexpr_cdr(struct sexpression * sexpr);

/**
 * Fetch the S-Expression object value as a string
 */
extern struct svalue * 
sexpr_value(struct sexpression * sexpr);

/**
 * Get the object sobj_get_type
 */
extern unsigned char
sexpr_type(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is NIL
 */
extern int
sexpr_is_nil(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is a CONS pair
 */
extern int
sexpr_is_cons(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is a string
 */
extern int
sexpr_is_value(struct sexpression *);

/**
 * Reverse a S-Expression list
 */
extern struct sexpression * 
sexpr_reverse(struct sexpression * sexpr);

/**
 * Return TRUE if both S-Expressions are equal
 */
extern int 
sexpr_equal(struct sexpression * a, struct sexpression * b);

#endif /* __SEXPR__H__ */
