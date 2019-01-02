#ifndef __SEXPR_STACK__H__
#define __SEXPR_STACK__H__

#include "stype.h"

/**
 * Push a S-Expression object into the top of the stack
 */
void
sexpr_push(struct sexpr ** , struct sobj * );

/**
 * Fetch the top most element of the stack
 */
struct sobj * 
sexpr_peek(struct sexpr ** );

/**
 * Remove and return the top-most element of the stack
 */
struct sobj *
sexpr_pop(struct sexpr ** );

/**
 * Returns TRUE if the stack can pop the top-most element, ie, is not empty.
 */
int
sexpr_can_pop(struct sexpr * );


#endif /* __SEXPR_STACK__H__ */
