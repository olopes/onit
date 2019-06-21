#include <stdlib.h>
#include "core_functions.h"

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);
static int
closure_arguments_are_valid(struct sexpression * closure_arguments);
static int 
bind_argument_values(struct sctx *, struct sexpression *, struct sexpression *);
static int 
bind_argument(struct sctx *, struct sexpression *, struct sexpression *);
static int 
bind_remaining_values(struct sctx *, struct sexpression *, struct sexpression *);

static enum sexpression_result 
_fn_lambda_compiler(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    enum sexpression_result return_value;
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if (!sexpr_is_cons(parameters)) {
        /* store descriptive error in result */
        return FN_ERROR;
    }
    
    if (closure_arguments_are_valid(sexpr_car(parameters))) {
        *result = alloc_new_function(sctx, _fn_lambda_evaluator, parameters);
        return_value = FN_OK;
    } else {
        /* store descriptive error in result */
        return_value = FN_ERROR;
    }
    
    return return_value;
}

static int
closure_arguments_are_valid(struct sexpression * closure_arguments) {
    struct sexpression * iter;
    struct sexpression * arg;
    struct sexpression * tail;
    
    switch(sexpr_type(closure_arguments)) {
    case ST_NIL:
    case ST_SYMBOL:
        return 1;
    case ST_CONS:
        break;
    default:
        /* store descriptive error in result */
        return 0;
    }
    
    iter = closure_arguments;
    while (1) {
        
        arg = sexpr_car(iter);
        if (!sexpr_is_nil(arg) && !sexpr_is_symbol(arg)) {
            /* store descriptive error in result */
            return 0;
        }
        
        /* last element might be a simple pair and not a list */
        
        if (sexpr_is_cons(iter)) {
            tail = sexpr_cdr(iter);
            if (sexpr_is_nil(tail)) {
                /* end of the list */
                break;
            } else if (sexpr_is_symbol(tail)) {
                /* this is a simple pair and CDR is symbol */
                break;
            } else if (sexpr_is_cons(tail)) {
                /* this is a list - evaluate the tail*/
                iter = tail;
            } else {
                /* store descriptive error in result */
                return 0;
            }
        } else {
            /* unexpected type - send pretty error */
            return 0;
        }
    }
    
    return 1;
}

static enum sexpression_result 
_fn_lambda_evaluator(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    enum sexpression_result return_value;
    
    if(sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if(result == NULL) {
        return FN_NULL_RESULT;
    }
    /* expand on bind */
    
    enter_namespace(sctx);
    
    if(bind_argument_values(sctx, sexpr_car(closure), parameters)) {
        /* write pretty error */
        return FN_ERROR;
    }
    
    return_value = fn_procedure(sctx, result, NULL, sexpr_cdr(closure));
    
    leave_namespace(sctx);
    
    return return_value;
}

static int 
bind_argument_values(struct sctx * sctx, struct sexpression * argument_declaration, struct sexpression * argument_values) {
    struct sexpression * decl_iter;
    struct sexpression * values_iter;
    struct sexpression * decl_tail;
    struct sexpression * values_tail;
    struct sexpression * argument;
    struct sexpression * value;
    
    if(argument_declaration == NULL) {
        return argument_values != NULL;
    }
    
    decl_iter = argument_declaration;
    values_iter = argument_values;
    
    while (1) {
        
        argument = sexpr_car(decl_iter);
        value = sexpr_car(values_iter);
        if (bind_argument(sctx, argument, value)) {
            /* something bad happens */
            return 1;
        }
        
        /* last element might be a simple pair and not a list */
        decl_tail = sexpr_cdr(decl_iter);
        values_tail = sexpr_cdr(values_iter);
        if (sexpr_is_nil(decl_tail) && sexpr_is_nil(values_tail)) {
            /* end of the list */
            break;
        } else if (sexpr_is_symbol(decl_tail)) {
            /* this is a simple pair and CDR is symbol */
            return bind_remaining_values(sctx, decl_tail, values_tail);
        } else if (sexpr_is_cons(decl_tail) && sexpr_is_cons(values_tail)) {
            /* this is a list - evaluate the tail*/
            decl_iter = decl_tail;
            values_iter = values_tail;
        } else {
            /* store descriptive error in result */
            return 1;
        }
    }
    
    return 0;
}

static int 
bind_argument(struct sctx * sctx, struct sexpression * argument_name, struct sexpression * argument_value) {
    struct mem_reference mem_ref;
    struct sexpression * value;
    
    if(sexpr_is_nil(argument_name)) {
        /* nothing to do */
        return 0;
    }
    
    if(!sexpr_is_symbol(argument_name)) {
        return 1;
    }
    
    if(fn_procedure_step(sctx, &value, argument_value) != FN_OK) {
        return 1;
    }
    
    if(create_stack_reference(sctx, sexpr_value(argument_name), sexpr_length(argument_name), &mem_ref)) {
        return 1;
    }
    
    *mem_ref.value = value;
    
    return 0;
}

static int 
bind_remaining_values(struct sctx * sctx, struct sexpression * argument_name, struct sexpression * arguments) {
    struct mem_reference mem_ref;
    struct sexpression * value;
    struct sexpression * iter;
    struct sexpression * values_list = NULL;
    
    iter = arguments;
    
    while(iter != NULL) {
        if(fn_procedure_step(sctx, &value, sexpr_car(iter)) != FN_OK) {
            return 1;
        }
        
        values_list = alloc_new_pair(sctx, value, values_list);
        
        iter = sexpr_cdr(iter);
    }
        
    values_list = sexpr_reverse(values_list);
    
    
    if(create_stack_reference(sctx, sexpr_value(argument_name), sexpr_length(argument_name), &mem_ref)) {
        return 1;
    }
    
    *mem_ref.value = values_list;
    
    return 0;
}



sexpression_callable fn_lambda = _fn_lambda_compiler;
