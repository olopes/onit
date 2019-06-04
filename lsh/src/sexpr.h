#ifndef __SEXPR__H__
#define __SEXPR__H__

#include <stddef.h>
#include <wchar.h>


/* object types */
enum sexpression_type { ST_NIL, ST_CONS, ST_SYMBOL, ST_STRING, ST_PRIMITIVE, ST_FUNCTION, ST_ERROR };

struct sctx;
struct sexpression;

enum sexpression_result {FN_OK, FN_NULL_SCTX, FN_NULL_RESULT, FN_ERROR};

/**
 * Primitive function interface.
 * Implementations should respect the following behavior
 * 
 * Parameters:
 *   sctx - vm context (should not be NULL)
 *   result - store resulting S-Expression in this address (should not be NULL)
 *   body - lambda function body
 *   parameters - arguments to pass to the primitive function
 * 
 * Returns:
 *   FN_OK - function finished successfuly
 *   FN_NULL_SCTX - sctx is NULL
 *   FN_NULL_RESULT - result is NULL
 *   FN_ERROR - An error has occurred. The result can be used to provide more details
 */
typedef enum sexpression_result
(*sexpression_callable)(
    struct sctx * sctx, 
    struct sexpression ** result,
    struct sexpression * closure, 
    struct sexpression * parameters);

struct sprimitive {
    void (*destructor)(void *);
    void (*print)(void *);
    void (*visit)(void *);
    void (*mark_reachable)(void *, unsigned char);
    int (*is_marked)(void *, unsigned char);
    int (*compare)(void *, void *);
};

struct sexpression { 
    size_t len;
    union sexpression_data {
        struct sexpression * sexpr;
        wchar_t * value;
        void * ptr;
        sexpression_callable function;
    } data;
    union sexpression_cdr {
        struct sexpression * sexpr;
        unsigned long hashcode;
        struct sprimitive * handler;
    } cdr;
    unsigned char visit_mark;
    enum sexpression_type type;
};

/**
 * Create a S-Expression pair (A . B)
 */
extern struct sexpression * 
sexpr_cons(struct sexpression * car, struct sexpression * cdr);

/**
 * Create a S-Expression symbol or a number
 */
extern struct sexpression * 
sexpr_create_symbol(wchar_t * cwstr, size_t length);

/**
 * Create a S-Expression string
 */
extern struct sexpression * 
sexpr_create_string(wchar_t * cwstr, size_t length);

/**
 * Create a S-Expression primitive
 */
extern struct sexpression *
sexpr_create_primitive(void * ptr, struct sprimitive * handler);

/**
 * Create a S-Expression pointing to a function
 */
extern struct sexpression *
sexpr_create_function (sexpression_callable function, struct sexpression * body);

/**
 * Create a S-Expression representing an error
 */
extern struct sexpression *
sexpr_create_error(wchar_t * wcstr, struct sexpression * call_stack);


/**
 * Release a S-Expression
 */
extern void
sexpr_free(struct sexpression * sexpr);

/**
 * Release a single S-Expression object without releasing any referenced objects
 */
extern void
sexpr_free_object(struct sexpression * sexpr);

/**
 * Return the S-Expression length
 */
extern size_t 
sexpr_length(struct sexpression * sexpr);

/**
 * Get the CAR part
 */
extern struct sexpression * 
sexpr_car(struct sexpression * sexpr);

/**
 * Get the CDR part
 */
extern struct sexpression * 
sexpr_cdr(struct sexpression * sexpr);

/**
 * Fetch the S-Expression object value as a string
 */
extern wchar_t * 
sexpr_value(struct sexpression * sexpr);

/**
 * Fetch the S-Expression object value as a string
 */
extern unsigned long 
sexpr_hashcode(struct sexpression * sexpr);

/**
 * Get S-Expression object pointer
 */
extern void *
sexpr_primitive_ptr(struct sexpression *sexpr);

/**
 * Get S-Expression handler pointer
 */
extern struct sprimitive *
sexpr_primitive_handler(struct sexpression *sexpr);

/**
 * Get S-Expression function pointer
 */
extern sexpression_callable 
sexpr_function(struct sexpression *sexpr);

/**
 * Get S-Expression function body
 */
extern struct sexpression *
sexpr_function_closure (struct sexpression *sexpr);

/**
 * Get the object sobj_get_type
 */
extern enum sexpression_type
sexpr_type(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is NIL
 */
extern int
sexpr_is_nil(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is a CONS pair
 */
extern int
sexpr_is_cons(struct sexpression * sexpr);

/**
 * Return TRUE if the given object is a symbol
 */
extern int
sexpr_is_symbol(struct sexpression *);

/**
 * Return TRUE if the given object is a string
 */
extern int
sexpr_is_string(struct sexpression *);

/**
 * Return TRUE if the given object is a pointer
 */
extern int
sexpr_is_primitive (struct sexpression *);

/**
 * Return TRUE if the given object is a function
 */
extern int
sexpr_is_function (struct sexpression *);

/**
 * Return TRUE if the given object is an error
 */
extern int
sexpr_is_error (struct sexpression *);

/**
 * Reverse a S-Expression list
 */
extern struct sexpression * 
sexpr_reverse(struct sexpression * sexpr);

/**
 * Return TRUE if both S-Expressions are equal
 */
extern int 
sexpr_equal(struct sexpression * a, struct sexpression * b);

/**
 * Set the visit mark for the given S-Expression and its references
 */
extern void
sexpr_mark_reachable (struct sexpression * sexpr, unsigned char visit_mark);

/**
 * Return TRUE if S-Expression visit mark is same as the parameter mark
 */
extern int
sexpr_is_marked (struct sexpression * sexpr, unsigned char visit_mark);

/**
 * Set the visit mark for the given S-Expression only
 */
extern void
sexpr_set_mark (struct sexpression * sexpr, unsigned char visit_mark);

/**
 * Compares two VALUE S-Expressions, returning value < 0 if a < b, value = 0 if a = b, value > 0 if a > b
 */
extern int
sexpr_compare(struct sexpression * a, struct sexpression * b);

#endif /* __SEXPR__H__ */
