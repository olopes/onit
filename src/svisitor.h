#ifndef __SVISITOR__H__
#define __SVISITOR__H__

#include <stdio.h>
#include "stype.h"

struct scallback {
    /**
     * Called when entering a CONS object
     */
    void (*enter) (struct sobj *, struct scallback *);
    /**
     * Called when visiting an object
     */
    void (*visit) (struct sobj *, struct scallback *);
    /**
     * Called when leaving a CONS object
     */
    void (*leave) (struct sobj *, struct scallback *);
    
    /**
     * Generic context
     */
    void * context;
};

/**
 * S-Expression visitor
 */
void 
svisitor(struct sobj *, struct scallback *);

/**
 * Dump S-Expression using svisitor
 */
void 
dump_sexpr(struct sexpr *, FILE *);

/**
 * Dump S-Expression Object using svisitor
 */
void 
dump_sobj(struct sobj *, FILE *);

/**
 * Dump S-Expression (recursive method)
 */
void 
dump_sexpr_r(struct sexpr *, FILE *);

/**
 * Dump S-Expression Object (recursive method)
 */
void 
dump_sobj_r(struct sobj * value, FILE * out);

#endif /* __VISITOR__H__ */
