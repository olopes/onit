#ifndef __EVAL__H__
#define __EVAL__H__

#include "sexpr.h"

extern struct sexpr * global_context;

/**
 * Eval a S-Expression Object
 */
struct sobj * 
eval_sexpr(struct sobj *);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding value
 */
struct sobj * 
eval_lookup_sym_value(struct sobj *name, struct sexpr *context);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding node
 */
struct sexpr * 
eval_lookup_sym_node(struct sobj *name, struct sexpr *context);


#endif /* __EVAL__H__ */

