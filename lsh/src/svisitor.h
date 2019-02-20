#ifndef __SVISITOR__H__
#define __SVISITOR__H__

#include <stdio.h>
#include "sexpr.h"

struct scallback {
    /**
     * Called when entering a CONS object
     */
    void (*enter) (struct sexpression *, struct scallback *);
    /**
     * Called when visiting an object
     */
    void (*visit) (struct sexpression *, struct scallback *);
    /**
     * Called when leaving a CONS object
     */
    void (*leave) (struct sexpression *, struct scallback *);
    
    /**
     * Generic context
     */
    void * context;
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
