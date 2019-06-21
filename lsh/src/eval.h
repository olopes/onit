#ifndef __EVAL__H__
#define __EVAL__H__

#include "sexpr.h"
#include "sctx.h"

/**
 * Eval a S-Expression Object
 */
extern enum sexpression_result
eval_sexpr(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression);


#endif /* __EVAL__H__ */

