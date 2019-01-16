#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "stype.h"
#include "sexpr.h"
#include "sobj.h"
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
    void (*fn) (struct sobj *, struct scallback *);
} fn_holder;

static void
empty_cb (struct sobj * obj, struct scallback * callback) {
    /* do nothing */
}

/**
 * S-Expression visitor
 */
void WEAK_FOR_UNIT_TEST
svisitor(struct sobj * obj, struct scallback * callback) {
    fn_holder cb_enter;
    fn_holder cb_visit;
    fn_holder cb_leave;
    struct sobj event_enter = {0 , &cb_enter, T_SYMBOL};
    struct sobj event_visit = {0 , &cb_visit, T_SYMBOL};
    struct sobj event_leave = {0 , &cb_leave, T_SYMBOL};
    struct sexpr * stack;
    struct sexpr * cons;
    struct sobj * value;
    struct sobj * event;
    
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
            /* handle value */
            cons = sobj_to_cons(value);
            
            /* insert leave event and add cdr */
            if(cons) {
                sexpr_push(&stack, value);
                sexpr_push(&stack, &event_leave);
                sexpr_push(&stack, sexpr_cdr(cons));
            }
            
            /* insert visit event */
            sexpr_push(&stack, value);
            sexpr_push(&stack, &event_visit);
            
            /* insert enter event and add car */
            if(cons) {
                sexpr_push(&stack, sexpr_car(cons));
                sexpr_push(&stack, value);
                sexpr_push(&stack, &event_enter);
            }
        }
    }
}


static void cb_enter(struct sobj *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    if(sobj_is_cons(obj)) {
        fputwc(L'(', out);
    }
}

static void cb_visit(struct sobj *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    switch(sobj_get_type(obj)) {
    case T_NIL:
        fputws(L"NIL", out);
        break;
    case T_STRING:
        fwprintf(out, L"\"%*ls\"", obj->len, (wchar_t *) obj->data);
        break;
    case T_SYMBOL:
        fwprintf(out, L"%*ls", obj->len, (wchar_t *) obj->data);
        break;
    case T_CONS:
        fputws(L" . ", out);
        break;
    default:
        break;
    }
}

static void cb_leave(struct sobj *obj, struct scallback *cb) {
    FILE * out = (FILE *) cb->context;
    
    if(obj == NULL) {
        return;
    }
    
    if(sobj_is_cons(obj)) {
        fputwc(L')', out);
    }
}

/**
 * Dump S-Expression
 */
void WEAK_FOR_UNIT_TEST
dump_sexpr(struct sexpr * sexpr, FILE * out) {
    struct sobj dummy;
    if(sexpr == NULL)
        return;
    
    dummy.data = sexpr;
    dummy.type = T_CONS;
    
    dump_sobj(&dummy, out);

}

/**
 * Dump S-Expression Object
 */
void WEAK_FOR_UNIT_TEST
dump_sobj(struct sobj * sobj, FILE * out) {
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
dump_sexpr_r(struct sexpr * sexpr, FILE * out) {
    struct sobj dummy;
    if(sexpr == NULL)
        return;
    
    dummy.data = sexpr;
    dummy.len  = -1;
    dummy.type = T_CONS;
    
    dump_sobj_r(&dummy, out);
}

/**
 * Dump S-Expression Object
 */
void WEAK_FOR_UNIT_TEST
dump_sobj_r(struct sobj * value, FILE * out) {
    struct sexpr * data;
    
    if(value == NULL) {
        return;
    }
    
    switch(sobj_get_type(value)) {
    case T_NIL:
        fputws(L"NIL", out);
        break;
    case T_STRING:
        fwprintf(out, L"\"%*ls\"", value->len, (wchar_t *) value->data);
        break;
    case T_SYMBOL:
        fwprintf(out, L"%*ls", value->len, (wchar_t *) value->data);
        break;
    case T_CONS:
        data = (struct sexpr *) value->data;
        fputwc(L'(', out);
        dump_sobj_r(data->car, out);
        fputws(L" . ", out);
        dump_sobj_r(data->cdr, out);
        fputwc(L')', out);
        break;
    }

}

