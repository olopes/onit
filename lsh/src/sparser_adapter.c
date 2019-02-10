#include <stdlib.h>
#include "sparser.h"

struct sparser_stream *
create_sparser_stream(const struct sparse_adapter * const adapter, ...) {
    /* mallocs and such */
    va_list ap;
    struct sparser_stream * stream;
    
    stream = (struct sparser_stream *) malloc(sizeof(struct sparser_stream));
    if(!stream) {
        return NULL;
    }
    stream->adapter = adapter;
    
    
    va_start(ap, adapter);
    
    if(adapter->init_stream(stream, ap)) {
        /* TODO handle error  */
        free(stream);
        stream = NULL;
    }
    
    va_end(ap);
    
    return stream;
}

void
release_sparser_stream(struct sparser_stream * stream) {
    
    stream->adapter->release_stream(stream);
    
    free(stream);
    
}

