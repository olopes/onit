#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include "sparser.h"
#include "cstr_sparser_adapter.h"

struct cstr_stream {
    char * data;
    size_t length;
    size_t position;
};

static int 
cstr_init_stream(struct sparser_stream * ctx, va_list ap) {
    struct cstr_stream * stream;
    char * fptr;
    size_t length;
    
    fptr = va_arg(ap, char *);
    length = va_arg(ap, size_t);
    
    if(!ctx || !fptr || length <= 0) {
        return 1;
    }
    
    stream = (struct cstr_stream *) malloc(sizeof(struct cstr_stream));
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
cstr_read_char(struct sparser_stream * ctx) {
    struct cstr_stream * stream;
    size_t chars_read;
    wchar_t chr;
    stream = (struct cstr_stream *) ctx->stream;
    
    if(stream->position >= stream->length) {
        return WEOF;
    }
        
    chars_read = mbtowc(&chr, stream->data, stream->length-stream->position);
    if(chars_read == -1) {
        /* handle error */
        return WEOF;
    }
    
    stream->position += chars_read;
    stream->data += chars_read;
    
    return chr;
}

static wint_t 
cstr_unread_char(struct sparser_stream * ctx, wint_t chr) {
    struct cstr_stream * stream;
    size_t chars_read;
    char chr_arr[MB_CUR_MAX];

    stream = (struct cstr_stream *) ctx->stream;
    
    if(stream->position <= 0 || chr == WEOF) {
        return WEOF;  /* which error ? */
    }
    
    chars_read = wctomb(chr_arr, (wchar_t)chr);
    if(chars_read == -1) {
        /* handle error */
        return WEOF;
    }
    
    stream->position -= chars_read;
    if(stream->position < 0) {
        /* oops... */
        stream->position = 0;
        return WEOF;
    }
    stream->data -= chars_read;
    
    return chr;
}

static int 
cstr_release_stream(struct sparser_stream * ctx) {
    if(ctx->stream) {
        free(ctx->stream);
    }
    return 0;
}

static struct sparse_adapter _CSTR_ADAPTER = {
    cstr_init_stream,
    cstr_read_char,
    cstr_unread_char,
    cstr_release_stream
};

const struct sparse_adapter * const CSTR_ADAPTER = &_CSTR_ADAPTER;

