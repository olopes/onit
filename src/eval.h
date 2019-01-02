#ifndef __EVAL__H__
#define __EVAL__H__

#include "sexpr.h"

extern struct sexpr * global_context;

/**
 * Eval a S-Expression Object
 */
struct sexpr_obj * 
eval_sexpr(struct sexpr_obj *);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding value
 */
struct sexpr_obj * 
eval_lookup_sym_value(struct sexpr_obj *name, struct sexpr *context);

/**
 * Search for a symbol in the context hierarchy and return the 
 * corresponding node
 */
struct sexpr * 
eval_lookup_sym_node(struct sexpr_obj *name, struct sexpr *context);


#endif /* __EVAL__H__ */

