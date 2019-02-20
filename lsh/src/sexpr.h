#ifndef __SEXPR__H__
#define __SEXPR__H__

#include <wchar.h>
#include "svalue.h"

enum stype {ST_NIL, ST_CONS, ST_VALUE};
enum scontent {SC_CONS, SC_STRING, SC_SYMBOL, SC_PROCEDURE, SC_NUMBER};

struct sexpression { 
    size_t len;
    void *data;
    enum stype type;
    enum scontent hint;
    struct sexpression * cdr;
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
 * Release a single S-Expression pair without releasing other components
 */
extern void
sexpr_free_pair(struct sexpression * sexpr);

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
extern enum stype
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
