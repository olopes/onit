#ifndef __SOBJ__H__
#define __SOBJ__H__

#include "stype.h"

/**
 * Global NIL object
 */
extern struct sobj * NIL;

/**
 * Create a S-Expression object from a string
 */
struct sobj * 
sobj_from_string(wchar_t *, size_t);

/**
 * Create a S-Expression object from a symbol
 */
struct sobj * 
sobj_from_symbol(wchar_t *, size_t);

/**
 * Create a S-Expression object from a S-Expression
 */
struct sobj * 
sobj_from_cons(struct sexpr *);

#define sobj_from_sexpr(p) sobj_from_cons(p)

/**
 * Release a S-Expression Object created by sobj_from_*
 */
void
sobj_free(struct sobj *);


/**
 * Fetch the S-Expression object value as a string
 */
struct sexpr_str * 
sobj_to_string(struct sobj *);

/**
 * Fetch the S-Expression object value as a symbol
 */
struct sexpr_str * 
sobj_to_symbol(struct sobj *);

/**
 * Fetch the S-Expression object value as a S-Expression
 */
struct sexpr * 
sobj_to_cons(struct sobj *);

#define sobj_to_sexpr(p) sobj_to_cons(p)

/**
 * Get the object sobj_get_type
 */
enum stype
sobj_get_type(struct sobj *);

/**
 * Return TRUE if the given object is NIL
 */
int
sobj_is_nil(struct sobj *);

/**
 * Return TRUE if the given object is a CONS pair
 */
int
sobj_is_cons(struct sobj *);

#define sobj_is_sexpr(p) sobj_is_cons(p)

/**
 * Return TRUE if the given object is a string
 */
int
sobj_is_string(struct sobj *);

/**
 * Return TRUE if the given object is a symbol
 */
int
sobj_is_symbol(struct sobj *);

#endif /* __SOBJ__H__ */
