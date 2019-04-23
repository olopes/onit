#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "svisitor.h"

/* syntatic sugar */
struct fn_holder {
    visitor_callback_fn fn;
};

static void cb_enter(struct sexpression *obj, struct scallback *cb);
static void cb_visit(struct sexpression *obj, struct scallback *cb);
static void cb_leave(struct sexpression *obj, struct scallback *cb);
static void empty_cb (struct sexpression * sexpr, struct scallback * cb);

void svisitor () __attribute__ ((weak, alias ("__svisitor")));

/**
 * S-Expression visitor
 */
void 
__svisitor(struct sexpression * obj, struct scallback * callback) {
    
    if(obj == NULL || callback == NULL) {
        return;
    }
    
    struct fn_holder cb_enter = { .fn = callback->enter ? callback->enter : &empty_cb };
    struct fn_holder cb_visit = { .fn = callback->visit ? callback->visit : &empty_cb };
    struct fn_holder cb_leave = { .fn = callback->leave ? callback->leave : &empty_cb };
    struct sexpression * stack;
    struct sexpression * value;
    struct sexpression * event;
    struct sexpression event_enter = {
        .len = 0, 
        .data = {.ptr = &cb_enter},
        .cdr = {.sexpr = NULL},
        .visit_mark = 0,
        .type = ST_PTR,
        .content = SC_PRIMITIVE,
    };
    struct sexpression event_visit = {
        .len = 0, 
        .data = {.ptr = &cb_visit},
        .cdr = {.sexpr = NULL},
        .visit_mark = 0,
        .type = ST_PTR,
        .content = SC_PRIMITIVE,
    };
    struct sexpression event_leave = {
        .len = 0, 
        .data = {.ptr = &cb_leave},
        .cdr = {.sexpr = NULL},
        .visit_mark = 0,
        .type = ST_PTR,
        .content = SC_PRIMITIVE,
    };
    
    stack = NULL;
    
    sexpr_push(&stack, obj);
    
    while(sexpr_can_pop(&stack)) {
        value = sexpr_pop(&stack);
        
        if(value == &event_enter || value == &event_visit || value == &event_leave) {
            /* handle event */
            event = value;
            value = sexpr_pop(&stack);
            ((struct fn_holder *) sexpr_ptr(event))->fn(value, callback);
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

static void
empty_cb (struct sexpression * sexpr, struct scallback * cb) {
    /* do nothing */
}

/**
 * Dump S-Expression
 */
void 
dump_sexpr(struct sexpression * sobj, FILE * out) {
    struct scallback print_callback = {
        .enter = &cb_enter,
        .visit = &cb_visit,
        .leave = &cb_leave,
        .context = out,
        .state = 0,
    };

    svisitor(sobj, &print_callback);

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
        fputws(L"()", out);
        break;
    case ST_VALUE:
        fwprintf(out, L"%*ls", obj->len, obj->data.value);
        break;
    case ST_CONS:
        fputws(L" . ", out);
        break;
    case ST_PTR:
        fwprintf(out, L"(ptr 0x%p)", obj->data.ptr);
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
void 
dump_sexpr_r(struct sexpression * obj, FILE * out) {    
    if(obj == NULL) {
        return;
    }
    
    switch(sexpr_type(obj)) {
    case ST_NIL:
        fputws(L"NIL", out);
        break;
    case ST_VALUE:
        fwprintf(out, L"%*ls", obj->len, obj->data.value);
        break;
    case ST_PTR:
        fwprintf(out, L"(ptr 0x%p)", obj->data.ptr);
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

