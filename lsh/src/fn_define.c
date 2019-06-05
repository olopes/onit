#include <stdlib.h>
#include "core_functions.h"
#include "sctx.h"
#include "eval.h"

static enum sexpression_result 
_fn_define(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * arguments );
static int arguments_are_invalid(struct sexpression * arguments);
static int get_name_symbol(struct sexpression * arguments, struct sexpression ** name);
static int compute_referenced_value(struct sctx * sctx, struct sexpression * arguments, struct sexpression ** value);


sexpression_callable fn_define = _fn_define;


static enum sexpression_result 
_fn_define(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * arguments ) {
    struct mem_reference mem_ref;
    struct sexpression * name_symbol;
    struct sexpression * referenced_value;
    
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
    
    if(compute_referenced_value(sctx, arguments, &referenced_value)) {
        *result = alloc_new_string(sctx, L"bad value", 9);
        return FN_ERROR;
    }
    
    if(create_global_reference (sctx, sexpr_value(name_symbol), sexpr_length(name_symbol), &mem_ref)) {
        *result = alloc_new_string(sctx, L"new definition failed", 21);
        return FN_ERROR;
    }
    
    *mem_ref.value = eval_sexpr(sctx, referenced_value);
        
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

static int compute_referenced_value(struct sctx * sctx, struct sexpression * arguments, struct sexpression ** value) {
    struct sexpression * head;
    struct sexpression * tail;
    
    head = sexpr_car( arguments );
    tail = sexpr_cdr( arguments );
    
    if (sexpr_is_symbol( head )) {
        *value = eval_sexpr(sctx, sexpr_car(tail));
    } else {
        // (define (a b c) ....) => (lambda (b c) ...)
        struct sexpression * lambda_symbol = alloc_new_symbol(sctx, L"lambda", 6);
        struct sexpression * function_arguments = sexpr_cdr(head);
        struct sexpression * function_body = tail;
        
        struct sexpression * function_definition = 
            alloc_new_pair(sctx, lambda_symbol, alloc_new_pair(sctx, function_arguments, function_body));
        
        *value = eval_sexpr(sctx, function_definition);
    }
    
    return 0;
}