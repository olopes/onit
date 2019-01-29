#ifndef __EVAL__H__
#define __EVAL__H__

#include "sexpr.h"

extern struct sexpr * global_context;

/**
 * Eval a S-Expression Object
 */
struct sexpression * 
eval_sexpr(struct sexpression *);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding value
 */
struct sexpression * 
eval_lookup_sym_value(struct sexpression *name, struct sexpression *context);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding node
 */
struct sexpression * 
eval_lookup_sym_node(struct sexpression *name, struct sexpression *context);


#endif /* __EVAL__H__ */

