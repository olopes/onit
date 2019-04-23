#ifndef __SVISITOR__H__
#define __SVISITOR__H__

#include <stdio.h>
#include "sexpr.h"

struct scallback;

typedef void (*visitor_callback_fn) (struct sexpression *, struct scallback *);

struct scallback {
    /**
     * Called when entering a CONS object
     */
    visitor_callback_fn enter;
    
    /**
     * Called when visiting an object
     */
    visitor_callback_fn visit;

    /**
     * Called when leaving a CONS object
     */
    visitor_callback_fn leave;
    
    /**
     * Generic context
     */
    void * context;
    
    /**
     * Generic state managed by the callback functions
     */
    int state;
};

/**
 * S-Expression visitor
 */
extern void 
svisitor(struct sexpression *, struct scallback *);

/**
 * Dump S-Expression using svisitor
 */
extern void 
dump_sexpr(struct sexpression *, FILE *);

/**
 * Dump S-Expression using recursive method
 */
extern void 
dump_sexpr_r(struct sexpression *, FILE *);

#endif /* __VISITOR__H__ */
