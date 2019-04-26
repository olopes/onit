#ifndef __EVAL__H__
#define __EVAL__H__

#include "sexpr.h"
#include "sctx.h"

/**
 * Eval a S-Expression Object
 */
extern struct sexpression * 
eval_sexpr(struct sctx *, struct sexpression *);


#endif /* __EVAL__H__ */

