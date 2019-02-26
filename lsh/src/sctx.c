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
};

#define MIN_OBJ 32
#define MAX_OBJ 131072

#define _INCLUDE_SCTX_C_

#include "sctx_primitives.c"

#include "sctx_alloc.c"

    
