#ifndef _SPARSE_PRIVATE_H_
#define _SPARSE_PRIVATE_H_
#include <wctype.h>
#include "sexpr.h"

struct sparse_ctx {
    struct sparser_stream *stream;
    wint_t prev;
    wint_t next;
};

static int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj);
static int 
sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj);
static int 
sparse_symbol(struct sparse_ctx * ctx, struct sexpression ** obj);
static int 
sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj);
static int 
sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj);
static int 
sparse_comment(struct sparse_ctx * ctx, struct sexpression * eoc);

#endif
