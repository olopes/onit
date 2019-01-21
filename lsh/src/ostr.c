#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "ostr.h"

/* 
NOTE TO SELF: this is a KISS implementation. I'll think about 
optimizations later.
*/


/* pre allocation size for dynamic growing ostr */
#define PREALLOC_SIZE 64

/**
 * Creates a new ostr node from a wchar_t string
 */
static struct ostr_token * 
ostr_new_node(wchar_t *str) {
    struct ostr_token *token;
    size_t len;
    
    token = (struct ostr_token *) malloc(sizeof(struct ostr_token));
    if(token == NULL && errno == ENOMEM) {
        return NULL;
    }
    
    if (str) {
        len = wcslen(str);
    } else {
        len = PREALLOC_SIZE;
    }
    
    token->str = (wchar_t *) calloc(len, sizeof(wchar_t));
    if(token->str == NULL && errno == ENOMEM) {
        free(token);
        return NULL;
    }
    
    token->size = len;
    token->length = 0;
    token->next = NULL;
    
    if (str) {
        token->length = len;
        memcpy(token->str, str, len * sizeof(wchar_t));
    } else {
        memset(token->str, 0, len * sizeof(wchar_t));
    }
    
    return token;
}    


/**
 * Creates a new ostr from a wchar_t struct
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_new(wchar_t *str) {
    struct ostr *ostr;
    
    ostr = (struct ostr *) malloc(sizeof(struct ostr));
    if(ostr == NULL && errno == ENOMEM) {
        return NULL;
    }
    
    ostr->first = ostr_new_node(str);
    if(ostr->first == NULL) {
        free(ostr);
        return NULL;
    }
    
    ostr->last = ostr->first;
    
    if (str) {
        ostr->length = ostr->first->length;
    } else {
        ostr->length = 0;
    }

    return ostr;
}    


/**
 * Destroys an existing ostr.
 */
void WEAK_FOR_UNIT_TEST
ostr_destroy(struct ostr * ostr) {
    struct ostr_token * token;
    struct ostr_token * next;
    
    if(ostr == NULL) {
        return;
    }
    
    token = ostr->first;
    while(token) {
        next = token->next;
        free(token->str);
        free(token);
        token = next;
    }
    
    free(ostr);
}

/**
 * Get the size (length) of an ostr
 */
size_t WEAK_FOR_UNIT_TEST
ostr_length(struct ostr * ostr) {
    return ostr->length;
}

/**
 * Append a wchar to an existing ostr
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_append(struct ostr * ostr, wchar_t chr) {
    struct ostr_token * token;
    
    if(ostr == NULL) {
        return NULL;
    }
    
    token = ostr->last;
    
    if(token->length >= token->size) {
        token->next = ostr_new_node(NULL);
        if(token->next == NULL && errno == ENOMEM) {
            return NULL;
        }
        
        ostr->last = token->next;
        token = token->next;
    }
    
    token->str[token->length] = chr;
    token->length++;
    ostr->length++;
    
    return ostr;
}


/**
 * Replace last wchar in the string with the provided wchar
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_replace_last(struct ostr * ostr, wchar_t chr) {
    struct ostr_token * token;
    
    if(ostr == NULL) {
        return NULL;
    }
    
    if(ostr->length == 0) {
        return NULL;
    }
    
    token = ostr->last;
    token->str[token->length-1] = chr;
    
    return ostr;
}


/**
 * Merge two ostrs. Similar to ostr_concat but this one is destructive.
 * The second ostr is reset to a zero size string.
 */
static struct ostr * 
ostr_merge(struct ostr * first, struct ostr * second) {
    
    if(first == second)
        return first;
    
    first->length += second->length;
    first->last->next = second->first;
    first->last = second->last;
    
    free(second);
    
    return first;
}


/**
 * Concatenate two ostrs
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_concat(struct ostr * first, struct ostr * second) {
    struct ostr *ostr;
    struct ostr *copy;
    
    ostr = ostr_dup(first);
    if(ostr == NULL && errno == ENOMEM) {
        return NULL;
    }
    
    copy = ostr_dup(second);
    if(copy == NULL && errno == ENOMEM) {
        ostr_destroy(ostr);
        return NULL;
    }
    
    return ostr_merge(ostr, copy);
}
    

/**
 * Compact a multi node ostr into a single node ostr.
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_compact(struct ostr *src) {
    struct ostr_token *token;
    struct ostr_token *next;
    wchar_t *str;
    
    str = ostr_str(src);
    if(str == NULL && errno == ENOMEM) {
        return NULL;
    }
    
    
    token = src->last = src->first;
    
    free(token->str);
    
    next = token->next;
    token->size = token->length = src->length;
    token->str = str;
    token->next = NULL;
    
    /* release remaining blocks */
    while(next) {
        token = next;
        next = token->next;
        free(token->str);
        free(token);
    }
        
    return src;
}


/**
 * Duplicates a ostr like compact but returns a new ostr instance
 */
struct ostr * WEAK_FOR_UNIT_TEST
ostr_dup(struct ostr * src) {
    struct ostr *ostr;
    struct ostr_token *token;
    
    ostr = (struct ostr *) malloc(sizeof(struct ostr));
    if(ostr == NULL && errno == ENOMEM) {
        return NULL;
    }
    
    token = (struct ostr_token *) malloc(sizeof(struct ostr_token));
    if(token == NULL && errno == ENOMEM) {
        free(ostr);
        return NULL;
    }
    
    token->size = token->length = ostr_length(src);
    token->next = NULL;
    token->str = ostr_str(src);
    if(token->str == NULL && errno == ENOMEM) {
        free(token);
        free(ostr);
        return NULL;
    }
    
    ostr->length = src->length;
    ostr->first = ostr->last = token;
    
    return ostr;
}

/**
 * Convert a ostr into a NULL terminated string
 */
wchar_t * WEAK_FOR_UNIT_TEST
ostr_str(struct ostr * ostr) {
    struct ostr_token * token;
    wchar_t * str;
    size_t position;
    
    str = (wchar_t *) malloc((ostr->length+1) * sizeof(wchar_t));
    if(str == NULL && errno == ENOMEM) {
        return NULL;
    }
    memset(str, 0, (ostr->length+1) * sizeof(wchar_t));
    
    position = 0;
    token = ostr->first;
    while(token) {
        memcpy(str+position, token->str, token->length * sizeof(wchar_t));
        position += token->length;
        token = token->next;
    }
    
    return str;
}


/**
 * Print a ostr to stdout
 */
int WEAK_FOR_UNIT_TEST
ostr_puts(struct ostr * ostr) {
    return ostr_fputs(ostr, stdout);
}

/**
 * Convert a ostr into a NULL terminated string
 */
int WEAK_FOR_UNIT_TEST
ostr_fputs(struct ostr * ostr, FILE * out) {
    struct ostr_token * token;
    int total;
    int i;

    total = 0;
    token = ostr->first;
    while(token) {
        for(i = 0; i < token->length; i++) {
            if(fputwc(token->str[i], out) == WEOF) {
                return total;
            }
            total++;
        }

        token = token->next;
    }
    
    return total;
}

/**
 * Get Nth character
 */
wint_t WEAK_FOR_UNIT_TEST
ostr_char_at(struct ostr * str, size_t position) {
    struct ostr_token * token;
    int start_pos;
    
    if(str == NULL) {
        return WEOF;
    }
    
    if(position >= str->length) {
        return WEOF;
    }
    
    if(position < 0) {
        return WEOF;
    }
    
    /* search for the correct token */
    token = str->first;
    start_pos = 0;
    while(token) {
        if(position >= start_pos && position < start_pos+token->length) {
            return token->str[position - start_pos];
        }
        start_pos += token->length;
        token = token->next;
    }
    
    return WEOF;
}
