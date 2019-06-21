#include "eval.h"
#include "sexpr.h"
#include "sexpr_stack.h"
#include "sctx.h"
#include "core_functions.h"

/**
 * Eval a S-Expression using the given context
 */
enum sexpression_result
eval_sexpr(struct sctx * sctx, struct sexpression ** result, struct sexpression * expression) {
    enum sexpression_result return_value;
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if (expression == NULL) {
        *result = NULL;
        return FN_OK;
    }
    
    return_value = fn_procedure_step(sctx, result, expression);
    
    return return_value;
    
}

