#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "svisitor.h"

/* TODO Implementar um visitor */
/*
Esse visitor recebe como parametros um objecto e um pointer para funcao.
Para pensar:
 - a funcao recebe um enum com {pre val pos} quando chamada antes de, durante, após visitar um nó;
 - em vez de funcao usar um struct com 3 pointers para funcao

Quando estiver concluido, implementar o dump usando o visitor.
Possibilita fazer um map e um reduce :-)

Para stack usar uma S-Expression.
*/
/* Maybe a SAX-y implementation? */

/* syntatic sugar */
typedef struct fn_holder {
    size_t len;
    void (*fn) (struct sexpression *, struct scallback *);
} fn_holder;

static void
empty_cb (struct sexpression * sexpr, struct scallback * callback) {
    /* do nothing */
}

/**
 * S-Expression visitor
 */
void WEAK_FOR_UNIT_TEST
svisitor(struct sexpression * obj, struct scallback * callback) {
    fn_holder cb_enter;
    fn_holder cb_visit;
    fn_holder cb_leave;
    struct sexpression * stack;
    struct sexpression * value;
    struct sexpression * event;
    struct sexpression event_enter = {
        .len = 0, 
        .data = &cb_enter,
        .cdr = NULL,
        .visit = 0,
        .type = ST_VALUE,
        .content = SC_PRIMITIVE,
    };
    struct sexpression event_visit = {
        .len = 0, 
        .data = &cb_visit,
        .cdr = NULL,
        .visit = 0,
        .type = ST_VALUE,
        .content = SC_PRIMITIVE,
    };
    struct sexpression event_leave = {
        .len = 0, 
        .data = &cb_leave,
        .cdr = NULL,
        .visit = 0,
        .type = ST_VALUE,
        .content = SC_PRIMITIVE,
    };
    
    if(obj == NULL || callback == NULL) {
        return;
    }
    
    cb_enter.fn = callback->enter ? callback->enter : &empty_cb;
    cb_visit.fn = callback->visit ? callback->visit : &empty_cb;
    cb_leave.fn = callback->leave ? callback->leave : &empty_cb;
    
    stack = NULL;
    
    sexpr_push(&stack, obj);
    
    while(sexpr_can_pop(stack)) {
        value = sexpr_pop(&stack);
        
        if(value == &event_enter || value == &event_visit || value == &event_leave) {
            /* handle event */
            event = value;
            value = sexpr_pop(&stack);
            ((fn_holder *) event->data)->fn(value, callback);
        } else {
            
            /* insert leave event and add cdr */
            if(sexpr_is_cons(value)) {
                sexpr_push(&stack, value);
                sexpr_push(&stack, &event_leave);
                sexpr_push(&stack, sexpr_cdr(value));
            }
            
            /* insert visit event */
            sexpr_push(&stack, value);
            sexpr_push(&stack, &event_visit);
            
            /* insert enter event and add car */
            if(sexpr_is_cons(value)) {
                sexpr_push(&stack, sexpr_car(value));
                sexpr_push(&stack, value);
                sexpr_push(&stack, &event_enter);
            }
        }
    }
}


static void cb_enter(struct sexpression *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    if(sexpr_is_cons(obj)) {
        fputwc(L'(', out);
    }
}

static void cb_visit(struct sexpression *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    switch(sexpr_type(obj)) {
    case ST_NIL:
        fputws(L"NIL", out);
        break;
    case ST_VALUE:
/*        fwprintf(out, L"\"%*ls\"", obj->len, (wchar_t *) obj->data);
        break;
    case T_SYMBOL:*/
        fwprintf(out, L"%*ls", obj->len, (wchar_t *) obj->data);
        break;
    case ST_CONS:
        fputws(L" . ", out);
        break;
    default:
        break;
    }
}

static void cb_leave(struct sexpression *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    if(sexpr_is_cons(obj)) {
        fputwc(L')', out);
    }
}

/**
 * Dump S-Expression
 */
void WEAK_FOR_UNIT_TEST
dump_sexpr(struct sexpression * sobj, FILE * out) {
    struct scallback print_callback = {
        &cb_enter,
        &cb_visit,
        &cb_leave,
        out
    };

    svisitor(sobj, &print_callback);

}


/**
 * Dump S-Expression
 */
void WEAK_FOR_UNIT_TEST
dump_sexpr_r(struct sexpression * obj, FILE * out) {    
    struct svalue * value;
    if(obj == NULL) {
        return;
    }
    
    switch(sexpr_type(obj)) {
    case ST_NIL:
        fputws(L"NIL", out);
        break;
/*    case ST_STRING:
        fwprintf(out, L"\"%*ls\"", value->len, (wchar_t *) value->data);
        break;*/
    case ST_VALUE:
        value = sexpr_value(obj);
        fwprintf(out, L"%*ls", value->len, value->data);
        break;
    case ST_CONS:
        fputwc(L'(', out);
        dump_sexpr_r(sexpr_car(obj), out);
        fputws(L" . ", out);
        dump_sexpr_r(sexpr_cdr(obj), out);
        fputwc(L')', out);
        break;
    }

}

