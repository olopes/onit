#ifndef __SEXPR__H__
#define __SEXPR__H__

#include "stype.h"

/**
 * Constructs a pair (A . B)
 */
struct sexpr * 
sexpr_cons(struct sobj * car, struct sobj * cdr);

/**
 * Release a S-Expression created by sexpr_cons
 */
void
sexpr_free(struct sexpr * sexpr);

/**
 * Get the CAR part
 */
struct sobj * 
sexpr_car(struct sexpr *);

/**
 * Get the CDR part
 */
struct sobj * 
sexpr_cdr(struct sexpr *);

#endif /* __SEXPR__H__ */
