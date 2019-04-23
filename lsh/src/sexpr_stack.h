#ifndef __SEXPR_STACK__H__
#define __SEXPR_STACK__H__

#include "sexpr.h"

/**
 * Push a S-Expression object into the top of the stack
 */
extern void
sexpr_push(struct sexpression ** , void * );

/**
 * Fetch the top most element of the stack
 */
extern void * 
sexpr_peek(struct sexpression ** );

/**
 * Remove and return the top-most element of the stack
 */
extern void *
sexpr_pop(struct sexpression ** );

/**
 * Returns TRUE if the stack can pop the top-most element, ie, is not empty.
 */
extern int
sexpr_can_pop(struct sexpression ** );


#endif /* __SEXPR_STACK__H__ */
