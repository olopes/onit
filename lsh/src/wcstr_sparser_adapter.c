#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#include "sparser.h"
#include "wcstr_sparser_adapter.h"

struct wcstr_stream {
    wchar_t * data;
    size_t length;
    size_t position;
};

static int 
wcstr_init_stream(struct sparser_stream * ctx, va_list ap) {
    struct wcstr_stream * stream;
    wchar_t * fptr;
    size_t length;
    
    fptr = va_arg(ap, wchar_t *);
    length = va_arg(ap, size_t);
    
    if(!ctx || !fptr || length < 0) {
        return 1;
    }
    
    stream = (struct wcstr_stream *) malloc(sizeof(struct wcstr_stream));
    if(!stream) {
        return 1;
    }
    
    stream->data = fptr;
    stream->length = length;
    stream->position = 0;
    
    ctx->stream = stream;
    
    return 0;
}

static wint_t 
wcstr_read_char(struct sparser_stream * ctx) {
    struct wcstr_stream * stream;
    wchar_t chr;
    stream = (struct wcstr_stream *) ctx->stream;
    
    if(stream->position >= stream->length) {
        return WEOF;
    }
        
    chr = *stream->data;
    
    stream->position++;
    stream->data++;
    
    return chr;
}

static wint_t 
wcstr_unread_char(struct sparser_stream * ctx, wint_t chr) {
    struct wcstr_stream * stream;
    stream = (struct wcstr_stream *) ctx->stream;
    
    if(stream->position <= 0 || chr == WEOF) {
        return WEOF;  /* which error ? */
    }
    stream->position--;
    stream->data--;
    /* decisions decisions... */
    /* *stream->data = (char_t) chr; */
    
    return chr;
}

static int 
wcstr_release_stream(struct sparser_stream * ctx) {
    if(ctx->stream) {
        free(ctx->stream);
    }
    return 0;
}

static struct sparse_adapter _WCSTR_ADAPTER = {
    wcstr_init_stream,
    wcstr_read_char,
    wcstr_unread_char,
    wcstr_release_stream
};

const struct sparse_adapter * const WCSTR_ADAPTER = &_WCSTR_ADAPTER;


