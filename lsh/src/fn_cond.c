#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "sexpr.h"
#include "core_functions.h"

enum cond_tristate {TS_FALSE, TS_TRUE, TS_ERROR};

static int 
validate_arguments_syntax (struct sctx *, struct sexpression *, struct sexpression **);

static enum cond_tristate 
evaluate_condition (struct sctx *, struct sexpression *, struct sexpression **, struct sexpression *);

static enum cond_tristate
is_condition_true (struct sctx *, struct sexpression *, struct sexpression *);

CoreFunction(cond) {
    struct mem_reference else_symbol_ref;
    struct sexpression * else_symbol;
    struct sexpression * ptr;
    enum cond_tristate cond_result; 
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if( validate_arguments_syntax (sctx, arguments, result)) {
        return FN_ERROR;
    }
    
    
    create_temporary_reference(sctx, &else_symbol_ref);
    *else_symbol_ref.value = else_symbol = alloc_new_symbol(sctx, L"else", 4);
    
    *result = NULL;
    
    cond_result = TS_FALSE;
    
    for(ptr = arguments; ptr != NULL; ptr = sexpr_cdr(ptr)) {
        struct sexpression * condition_group = sexpr_car(ptr);
        cond_result = evaluate_condition(sctx, condition_group, result, else_symbol);
        if(cond_result != TS_FALSE) {
            break;
        }
    }
    
    
    return cond_result == TS_ERROR ? FN_ERROR : FN_OK;
}

static int
validate_arguments_syntax (struct sctx * sctx, struct sexpression * arguments, struct sexpression ** result) {
    struct sexpression * ptr;
    
    if(!sexpr_is_cons(arguments)) {
        // pretty error message
        return 1;
    }
    
    for(ptr = arguments; ptr != NULL; ptr = sexpr_cdr(ptr)) {
        struct sexpression * condition_group = sexpr_car(ptr);
        
        if(!sexpr_is_cons(condition_group)) {
            // bad group - not a list
            return 1;
        }
        
        if(sexpr_length(condition_group) < 2) {
            // bad group - list too small
            return 1;
        }
        
    }
    
    return 0;
}


static enum cond_tristate 
evaluate_condition(struct sctx * sctx, struct sexpression * condition_group, struct sexpression ** result, struct sexpression * else_symbol) {
    struct sexpression * condition = sexpr_car(condition_group);
    struct sexpression * condition_body = sexpr_cdr(condition_group);
    
    enum cond_tristate cond_eval_state = is_condition_true(sctx, condition, else_symbol);
    
    if(cond_eval_state == TS_ERROR) {
        /* log a nice error? */
        return TS_ERROR;
    }
    
    if(cond_eval_state == TS_TRUE) {
        return fn_procedure(sctx, result, NULL, condition_body) == FN_OK ? TS_TRUE : TS_ERROR;
    } 
    
    return TS_FALSE;
}

static enum cond_tristate
is_condition_true(struct sctx * sctx, struct sexpression * condition, struct sexpression * else_symbol) {
    struct sexpression * return_value;
    if(sexpr_equal(condition, else_symbol)) {
        return TS_TRUE;
    }
    
    if(fn_procedure_step(sctx, &return_value, condition) != FN_OK) {
        // assumes false if error occurs
        return TS_ERROR;
    }
    
    /* if symbol doesn't exist, it's returned by fn_procedure_step. */
    if(sexpr_is_nil(return_value) || sexpr_is_symbol(return_value)) {
        return TS_FALSE;
    }
    
    
    return TS_TRUE;
}
