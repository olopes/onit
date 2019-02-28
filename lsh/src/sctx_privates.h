#ifndef __SCTX_PRIVATES_H__
#define __SCTX_PRIVATES_H__

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "sexpr_stack.h"
#include "shash.h"
#include "aa_tree.h"
#include "sctx.h"


struct sctx {
    struct shash_table primitives;
    struct sexpression * namespaces;
    struct sexpression * environment;
    struct sexpression * arguments;
    int visit;
    int heap_size;
    int heap_load;
    struct sexpression ** heap;
    void (*namespace_destructor)(struct sctx * ctx);
};

#define HEAP_MIN_SIZE 32
#define HEAP_MAX_SIZE 131072

#endif
    
