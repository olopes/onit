#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include "sexpr.h"
#include "core_functions.h"

static enum sexpression_result 
_fn_echo(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);

static size_t
print_sexpression(struct sctx * sctx, struct sexpression * input, size_t printed);
static void write_string(struct sctx * sctx, wchar_t * str, size_t length);

sexpression_callable fn_echo = _fn_echo;

static enum sexpression_result 
_fn_echo(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    struct sexpression * iter = parameters;
    
    if (sctx == NULL) {
        return FN_NULL_SCTX;
    }
    
    if (result == NULL) {
        return FN_NULL_RESULT;
    }
    
    size_t printed = 0;
    while(iter != NULL) {
        printed = print_sexpression(sctx, sexpr_car(iter), printed);
        
        iter = sexpr_cdr(iter);
    }
    
    struct sexpression * new_line = lookup_name(sctx, alloc_new_symbol(sctx, L"#nl", 3));
    write_string(sctx, sexpr_value(new_line), sexpr_length(new_line));
    
    *result = NULL;
    return FN_OK;
}

static size_t
print_sexpression(struct sctx * sctx, struct sexpression * input, size_t printed) {
    struct sexpression * value = input;
    size_t length;
    wchar_t * str;
    
    while(sexpr_is_symbol(value)) {
        struct sexpression * bound_value = lookup_name(sctx, value);
        if(bound_value == NULL) {
            break;
        }
        value = bound_value;
    }
    
    if (sexpr_is_symbol(value) || sexpr_is_string(value)) {
        length = sexpr_length(value);
        str = sexpr_value(value);
        if(printed && length > 0) {
            sctx->print_char(L' ');
            printed += 1;
        }
        write_string(sctx, str, length);
        printed += length;
    }
    return printed;
}

static void write_string(struct sctx * sctx, wchar_t * str, size_t length) {
    if(NULL == str || length == 0) {
        return;
    }
    for(size_t i = 0; i < length; i++, str++) {
        if(sctx->print_char(*str) == WEOF) {
            break;
        }
    }
}
