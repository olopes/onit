#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sexpr.h"
#include "sexpr_stack.h"

static inline size_t stmin(size_t a, size_t b); 
static unsigned long compute_hashcode(wchar_t * value, size_t len);
static struct sexpression *
sexpr_create_value(wchar_t * cstr, size_t len, enum sexpression_type type);
static void print_fn_ptr(void * ptr);
static int compare_fn_ptr(void * a, void * b);


/**
 * Constructs a pair (A . B)
 */
struct sexpression * 
sexpr_cons(struct sexpression * car, struct sexpression * cdr) {
    struct sexpression * sexpr;
    
    sexpr = (struct sexpression *) malloc(sizeof(struct sexpression));
    if(sexpr == NULL) {
        return NULL;
    }
    
    memset(sexpr, 0, sizeof(struct sexpression));
    sexpr->type = ST_CONS;
    sexpr->data.sexpr = car;
    sexpr->cdr.sexpr = cdr;
    
    if(sexpr_is_cons(cdr)) {
        sexpr->len = cdr->len + 1;
    } else if(sexpr_is_nil(cdr)) {
        sexpr->len = 1;
    } else {
        sexpr->len = 0;
    }
    
    return sexpr;
}

/**
 * Create a S-Expression symbol
 */
struct sexpression *
sexpr_create_symbol(wchar_t * cstr, size_t len) {
    return sexpr_create_value(cstr, len, ST_SYMBOL);
}

/**
 * Create a S-Expression quoted string
 */
struct sexpression *
sexpr_create_string(wchar_t * cstr, size_t len) {
    return sexpr_create_value(cstr, len, ST_STRING);
}

/**
 * Create a S-Expression value
 */
static struct sexpression *
sexpr_create_value(wchar_t * cstr, size_t len, enum sexpression_type type) {
    struct sexpression * sexpr;
    
    if(cstr == NULL) {
        return NULL;
    }
    
    sexpr = sexpr_cons(NULL, NULL);
    if(sexpr == NULL) {
        return NULL;
    }
    
    sexpr->type = type;
    sexpr->len = len;
    sexpr->data.value = malloc(sizeof(wchar_t)*(len+1));
    if(sexpr->data.value == NULL) {
        sexpr_free(sexpr);
        return NULL;
    }
    memcpy(sexpr->data.value, cstr, sizeof(wchar_t)*len);
    sexpr->data.value[len] = L'\0';
    sexpr->cdr.hashcode = compute_hashcode(sexpr->data.value, len);
    
    return sexpr;
}

/**
 * Create a S-Expression primitive
 */
struct sexpression *
sexpr_create_primitive(void * ptr, struct sprimitive * handler) {
    struct sexpression * sexpr;
    
    if(handler == NULL) {
        return NULL;
    }
    
    sexpr = sexpr_cons(NULL, NULL);
    if(sexpr == NULL) {
        return NULL;
    }
    
    sexpr->type = ST_PRIMITIVE;
    sexpr->len = 0;
    sexpr->data.ptr = ptr;
    sexpr->cdr.handler = handler;
    
    return sexpr;
}

struct sprimitive function_handler = {
    .destructor = NULL,
    .print = print_fn_ptr,
    .print = NULL,
    .visit = NULL,
    .mark_reachable = NULL,
    .is_marked = NULL,
    .compare = compare_fn_ptr
};

/**
 * Create a S-Expression function pointer
 */
struct sexpression *
sexpr_create_function(sexpression_callable function) {
    struct sexpression * sexpr;
    
    sexpr = sexpr_create_primitive(NULL, &function_handler);
    if(sexpr == NULL) {
        return NULL;
    }
    
    sexpr->type = ST_FUNCTION;
    sexpr->data.function = function;
    
    return sexpr;
}

void
sexpr_free(struct sexpression * sexpr) {
    if(sexpr == NULL) {
        return;
    }
    
    if(sexpr_is_cons(sexpr)) 
    {
        sexpr_free(sexpr_car(sexpr));
        sexpr_free(sexpr_cdr(sexpr));
    }
    sexpr_free_object(sexpr);
}

void
sexpr_free_object(struct sexpression * sexpr) {
    if(sexpr == NULL) {
        return;
    }
    
    if(sexpr_is_string(sexpr) || sexpr_is_symbol(sexpr)) {
        free(sexpr->data.value);
    } else if(sexpr_is_primitive(sexpr) && sexpr_primitive_handler(sexpr)->destructor != NULL) {
        sexpr_primitive_handler(sexpr)->destructor(sexpr_primitive_ptr(sexpr));
    }


    free(sexpr);
}

size_t
sexpr_length(struct sexpression * sexpr) {
    return sexpr == NULL ? 0 : sexpr->len;
}
/**
 * Get the CAR part
 */
struct sexpression *
sexpr_car(struct sexpression * sexpr) {
    /* how to raise error? */
    return sexpr_type(sexpr) == ST_CONS ? sexpr->data.sexpr : NULL;
}

/**
 * Get the CDR part
 */
struct sexpression *
sexpr_cdr(struct sexpression * sexpr) {
    /* how to raise error? */
    return sexpr_type(sexpr) == ST_CONS ? sexpr->cdr.sexpr : NULL;
}

/**
 * Get S-Expression value 
 */
wchar_t *
sexpr_value(struct sexpression *sexpr) {
    enum sexpression_type type = sexpr_type(sexpr);
    return type == ST_SYMBOL || type == ST_STRING  ? sexpr->data.value : NULL;
}

/**
 * Get S-Expression hash code
 */
unsigned long
sexpr_hashcode(struct sexpression *sexpr) {
    enum sexpression_type type = sexpr_type(sexpr);
    return type == ST_SYMBOL || type == ST_STRING ? sexpr->cdr.hashcode : 0;
}

/**
 * Get S-Expression object pointer
 */
void *
sexpr_primitive_ptr(struct sexpression *sexpr) {
    return sexpr_type(sexpr) == ST_PRIMITIVE ? sexpr->data.ptr : NULL;
}

/**
 * Get S-Expression handler pointer
 */
struct sprimitive *
sexpr_primitive_handler(struct sexpression *sexpr) {
    return sexpr_type(sexpr) == ST_PRIMITIVE ? sexpr->cdr.handler : NULL;
}

/**
 * Get S-Expression handler pointer
 */
sexpression_callable
sexpr_function(struct sexpression *sexpr) {
    return sexpr_type(sexpr) == ST_FUNCTION ? sexpr->data.function : NULL;
}

/**
 * Get the S-Expression type
 */
enum sexpression_type
sexpr_type(struct sexpression * sexpr) {
    return sexpr == NULL ? ST_NIL : sexpr->type ;
}

/**
 * Return TRUE if the given object is NIL
 */
int
sexpr_is_nil(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_NIL;
}

/**
 * Return TRUE if the given object is a CONS pair
 */
int
sexpr_is_cons(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_CONS;
}

/**
 * Return TRUE if the given object is a string
 */
int
sexpr_is_string(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_STRING;
}

/**
 * Return TRUE if the given object is a symbol
 */
int
sexpr_is_symbol(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_SYMBOL;
}

/**
 * Return TRUE if the given object is a primitive
 */
int
sexpr_is_primitive(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_PRIMITIVE;
}

/**
 * Return TRUE if the given object is a function
 */
int
sexpr_is_function(struct sexpression * sexpr) {
    return sexpr_type(sexpr) == ST_FUNCTION;
}

/**
 * Return TRUE if both S-Expressions are equal
 */
int sexpr_equal(struct sexpression * a, struct sexpression * b) {
    int are_equal;
    
    /* same object ? */
    if(a == b) {
        are_equal = 1;
    } else if(a == NULL || b == NULL) {
        are_equal = 0;
    } else if (sexpr_type(a) != sexpr_type(b)) {
        are_equal = 0;
    } else if (sexpr_is_cons(a)) {
        are_equal = (sexpr_equal(sexpr_car(a), sexpr_car(b)) && sexpr_equal(sexpr_cdr(a), sexpr_cdr(b)));
    } else if ( sexpr_is_primitive (a) || sexpr_is_function(a)) {
        are_equal = (a->data.ptr == b->data.ptr) && (a->cdr.handler == b->cdr.handler);
    } else {
        are_equal = a->len == b->len && wcsncmp((wchar_t *) a->data.value, (wchar_t *) b->data.value, a->len) == 0;
    }
    return are_equal;
}

/**
 * Reverse a S-Expression list
 */
struct sexpression * 
sexpr_reverse(struct sexpression * sexpr) {
    struct sexpression * car;
    struct sexpression * cdr;
    struct sexpression * iter;
    struct sexpression * reversed;
    size_t length;
    
    if(!sexpr_is_cons(sexpr)) {
        return sexpr;
    }
    
    car = sexpr_car(sexpr);
    cdr = sexpr_cdr(sexpr);
    
    /* handle a "list" with size 1*/
    if(sexpr_is_nil(cdr)) {
        sexpr->len = 1;
        return sexpr;
    }
    
    /* handle a pair */
    if(!sexpr_is_cons(cdr)) {
        sexpr->data.sexpr = cdr;
        sexpr->cdr.sexpr = car;
        return sexpr;
    }
    
    /* handle a "list" */
    
    length = 0;
    reversed = NULL;
    iter = sexpr;
    while(iter) {
        length++;
        iter->len = length;
        cdr = sexpr_cdr(iter);
        
        /* manipulate pointers */
        iter->cdr.sexpr = reversed;
        reversed = iter;
        iter = cdr;
    }
    
    return reversed;
}



void
sexpr_mark_reachable(struct sexpression * object, unsigned char visit_mark) {
    struct sexpression * sexpr = object;
    
    while(sexpr != NULL) {
        sexpr->visit_mark = visit_mark;
        sexpr_mark_reachable(sexpr_car(sexpr), visit_mark);
        sexpr = sexpr_cdr(sexpr);
    }
    
}

int
sexpr_is_marked (struct sexpression * sexpr, unsigned char visit_mark) {
    return sexpr != NULL && sexpr->visit_mark == visit_mark;
}

void
sexpr_set_mark (struct sexpression * sexpr, unsigned char visit_mark) {
    if (sexpr == NULL) return;
    sexpr->visit_mark = visit_mark;
}


/* djb2 implementation */
static unsigned long compute_hashcode(wchar_t * value, size_t len) {
    unsigned long hash;
    char * ptr = (char*) value;
    const size_t num_bytes = len * sizeof(wchar_t);
    size_t i;
    
    
    hash = 5381;
    
    for(i = 0; i < num_bytes; i++, ptr++) {
        hash = ((hash << 5) + hash) + *ptr; /* hash * 33 + c */
    }
    return hash;
}

int sexpr_compare(struct sexpression * a, struct sexpression * b) {
    size_t len;
    int cmp;
    if(a == b) {
        cmp = 0;
    } else if(a == NULL) {
        cmp = -1;
    } else if(b == NULL) {
        cmp = 1;
    } else {
        len = stmin(a->len, b->len);
        cmp = wmemcmp(a->data.value, b->data.value, len);
        if(cmp == 0) {
            cmp = a->len - b->len;
        }
    }
    
    return cmp;
}

static inline size_t stmin(size_t a, size_t b) {
    return a > b ? b : a;
}

static void print_fn_ptr(void * ptr) {
    wprintf(L"#function %p", ptr);
}

static int compare_fn_ptr(void * a, void * b) {
    return a == b ? 0 : (a> b ? 1 : -1); 
}
