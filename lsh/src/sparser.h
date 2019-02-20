#ifndef __SPARSER_H__
#define __SPARSER_H__

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include "sexpr.h"

#define SPARSE_OK 0
#define SPARSE_EOF 1
#define SPARSE_BAD_CHAR 2
#define SPARSE_PAREN 3
#define SPARSE_BAD_SYM 4
#define SPARSE_DOT_SYM 5

struct sparser_stream;

struct sparse_adapter {
    
    int (*init_stream)(struct sparser_stream *, va_list ap);

    wint_t (*read_char)(struct sparser_stream *);
    
    wint_t (*unread_char)(struct sparser_stream *, wint_t);
    
    int (*release_stream)(struct sparser_stream *);
};

struct sparser_stream {
    void * stream;
    const struct sparse_adapter * adapter;
};

/**
 * Creates a new stream from the provided adapter
 */
extern struct sparser_stream *
create_sparser_stream(const struct sparse_adapter * const adapter, ...);

/**
 * Release stream resources
 */
extern void
release_sparser_stream(struct sparser_stream *);


/**
 * Read (parse) a S-Expression object from the specified stream 
 */
extern int
sparse(struct sparser_stream * stream, struct sexpression ** parsed_object);

#endif /* __SPARSER_H__ */
