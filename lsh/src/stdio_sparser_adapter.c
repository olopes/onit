#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include <stdarg.h>
#include "sparser.h"
#include "stdio_sparser_adapter.h"


static int 
stdio_init_stream(struct sparser_stream *ctx, va_list ap) {
    FILE * fptr = va_arg(ap, FILE *);
    
    if(!ctx || !fptr) {
        return 1;
    }
    
    ctx->stream = fptr;
    
    return 0;
}

static wint_t 
stdio_read_char(struct sparser_stream * ctx) {
    return fgetwc((FILE *)ctx->stream);
}

static wint_t 
stdio_unread_char(struct sparser_stream * ctx, wint_t chr) {
    return ungetwc(chr, (FILE *)ctx->stream);
}

static int 
stdio_release_stream(struct sparser_stream * ctx) {
    /* nothing to do */
    return 0;
}


static int 
stdio_init_file_stream(struct sparser_stream *ctx, va_list ap) {
    char * file_name;
    FILE * fptr;
    
    file_name = va_arg(ap, char*);
    
    if(!ctx || !file_name) {
        return 1;
    }
    
    
    fptr = fopen(file_name, "r");
    if(!fptr) {
        /* handle fopen error */
        return 1;
    }
    
    ctx->stream = fptr;
    
    return 0;
}

static int 
stdio_release_file_stream(struct sparser_stream * ctx) {
    return fclose((FILE*)ctx->stream);
}


static struct sparse_adapter _FILE_DESCRIPTOR_ADAPTER = {
    stdio_init_stream,
    stdio_read_char,
    stdio_unread_char,
    stdio_release_stream
};

static struct sparse_adapter _FILE_NAME_ADAPTER = {
    stdio_init_file_stream,
    stdio_read_char,
    stdio_unread_char,
    stdio_release_file_stream
};

const struct sparse_adapter * const FILE_DESCRIPTOR_ADAPTER = &_FILE_DESCRIPTOR_ADAPTER;

const struct sparse_adapter * const FILE_NAME_ADAPTER = &_FILE_NAME_ADAPTER;
