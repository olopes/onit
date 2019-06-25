#include <stdlib.h>
#include "core_functions.h"
#include "sctx.h"
#include "eval.h"

static enum sexpression_result 
_fn_define(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * arguments );
static int arguments_are_invalid(struct sexpression * arguments);
static int get_name_symbol(struct sexpression * arguments, struct sexpression ** name);
static enum sexpression_result
compute_referenced_value(struct sctx * sctx, struct sexpression * arguments, struct sexpression ** value);
static int
store_value_in_context(struct sctx * sctx, struct sexpression * name_symbol, struct sexpression * value);
static int 
create_lambda_expression (struct sctx * sctx, struct sexpression ** expression, struct sexpression * definition, struct sexpression * function_body);

sexpression_callable fn_define = _fn_define;

static enum sexpression_result 
_fn_define(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * arguments ) {
    struct sexpression * name_symbol;
    enum sexpression_result return_value;
    
    if(sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if(result == NULL) {
        return FN_NULL_RESULT;
    }
    
    if(arguments_are_invalid(arguments)) {
        *result = alloc_new_string(sctx, L"bad syntax", 10);
        return FN_ERROR;
    }
    
    if(get_name_symbol(arguments, &name_symbol)) {
        *result = alloc_new_string(sctx, L"bad symbol name", 15);
        return FN_ERROR;
    }
    
    return_value = compute_referenced_value(sctx, arguments, result);
    if(return_value != FN_OK) {
        return return_value;
    }
    
    if(store_value_in_context(sctx, name_symbol, *result)) {
        *result = alloc_new_string(sctx, L"new definition failed", 21);
        return FN_ERROR;
    }
    
    return FN_OK;
}

static int arguments_are_invalid(struct sexpression * arguments) {
    struct sexpression * head;
    struct sexpression * tail;
    
    if(sexpr_is_nil(arguments)) {
        return 1;
    }
    
    head = sexpr_car( arguments );
    tail = sexpr_cdr( arguments );
    
    if(!sexpr_is_cons( tail )) {
        return 1;
    }
    
    // TODO refactor if body
    if (sexpr_is_symbol( head )) {
        // validate symbol
        if (sexpr_length( tail ) != 1) {
            // prepare a fancy error
            return 1;
        }
    } else if(sexpr_is_cons( head )) {
        // validate lambda
        struct sexpression * symbol_name = sexpr_car(head);
        
        if (!sexpr_is_symbol( symbol_name )) {
            return 1;
        }
        
    } else {
        return 1;
    }
    
    return 0;
}

static int get_name_symbol(struct sexpression * arguments, struct sexpression ** name) {
    struct sexpression * head;
    
    head = sexpr_car( arguments );
    
    if(sexpr_is_symbol(head)) {
        *name = head;
    } else {
        *name = sexpr_car(head);
    }
    
    return 0;
}

static enum sexpression_result
compute_referenced_value(struct sctx * sctx, struct sexpression * arguments, struct sexpression ** value) {
    struct sexpression * head;
    struct sexpression * tail;
    struct sexpression * expression;
    
    head = sexpr_car( arguments );
    tail = sexpr_cdr( arguments );
    
    if (sexpr_is_symbol( head )) {
        expression = sexpr_car(tail);
    } else {
        create_lambda_expression (sctx, &expression, head, tail);
    }
    
    return fn_procedure_step(sctx, value, expression);
}

static int
store_value_in_context(struct sctx * sctx, struct sexpression * name_symbol, struct sexpression * value) {
    struct mem_reference mem_ref;
    if(create_global_reference (sctx, sexpr_value(name_symbol), sexpr_length(name_symbol), &mem_ref)) {
        return 1;
    }
    *mem_ref.value = value;
    return 0;
}

// transform (define (a b c) ....) into (lambda (b c) ...)
static int 
create_lambda_expression (struct sctx * sctx, struct sexpression ** expression, struct sexpression * definition, struct sexpression * function_body) {
    struct mem_reference lambda_reference;
    struct mem_reference body_reference;
    struct sexpression * function_arguments = sexpr_cdr( definition );
    
    /* create temporary references to create the new lambda expression */
    if (create_temporary_reference(sctx, &lambda_reference)) {
        return 1;
    }
    if (create_temporary_reference(sctx, &body_reference)) {
        return 1;
    }
    
    *lambda_reference.value = alloc_new_symbol(sctx, L"lambda", 6);
    if(*lambda_reference.value == NULL) {
        return 1;
    }
    
    *body_reference.value = alloc_new_pair(sctx, function_arguments, function_body);
    if(*body_reference.value == NULL) {
        return 1;
    }
    
    *body_reference.value = alloc_new_pair(sctx, *lambda_reference.value, *body_reference.value);
    if(*body_reference.value == NULL) {
        return 1;
    }
    
    *expression = *body_reference.value;
    
    return 0;
}
