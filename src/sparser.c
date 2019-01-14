#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include "sparser.h"
#include "ostr.h"
#include "sobj.h"
#include "sexpr.h"

/*
From Racket docs:
Any string (i.e., any character sequence) can be supplied to string->symbol to obtain the corresponding symbol. For reader input, any character can appear directly in an identifier, except for whitespace and the following special characters:

   ( ) [ ] { } " , ' ` ; # | \

Actually, # is disallowed only at the beginning of a symbol, and then only if not followed by %; otherwise, # is allowed, too. Also, . by itself is not a symbol.

Whitespace or special characters can be included in an identifier by quoting them with | or \. These quoting mechanisms are used in the printed form of identifiers that contain special characters or that might otherwise look like numbers.

; is a comment
*/

struct sparse_ctx {
    FILE *in;
    wint_t prev;
    wint_t next;
    struct sexpr * stack;
};

int 
sparse_object(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_string(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_simple_symbol(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_quote(struct sparse_ctx * ctx, struct sobj ** obj);
int 
sparse_cons(struct sparse_ctx * ctx, struct sobj ** obj);


int WEAK_FOR_UNIT_TEST
sparse(FILE *in, struct sobj ** obj) {
    struct sparse_ctx ctx;
    ctx.in = in;
    ctx.next = L' ';
    ctx.stack = NULL;

    sparse_object(&ctx, obj);
    
    /* be careful! it might be WEOF or no read at all! */
    ungetwc(ctx.next, in);

    return 0;
}

int WEAK_FOR_UNIT_TEST
sparse_object(struct sparse_ctx * ctx, struct sobj ** obj) {
    struct sobj * ostr;
    int ret_val;
    ret_val = SPARSE_OK;
    ostr = NULL;
    
    while(1) {
        ctx->prev = ctx->next;
        ctx->next = fgetwc(ctx->in);
        if(ctx->next == WEOF) {
            ret_val = SPARSE_EOF;
            break;
        }

        if(iswspace(ctx->next)) {
            continue;
        } else {

            if(ctx->next == L'"') {
                // start or end str
                ret_val = sparse_string(ctx, &ostr);
            } else if(ctx->next == L'|') {
                /* start or end symbol */
                ret_val = sparse_symbol(ctx, &ostr);
            } else if(ctx->next == L'(') {
                /* start cons */
                ret_val = sparse_cons(ctx, &ostr);
            } else if(ctx->next == L'\'') {
                /* 
                start quote - the quote ends with the current expression. 
                translates to (quote xxxx)
                */
                ret_val = sparse_quote(ctx, &ostr);
            } else {
                ret_val = sparse_simple_symbol(ctx, &ostr);
            }
            break;
        }
    }
    
    *obj = ostr;
    
    return ret_val;
}


int WEAK_FOR_UNIT_TEST
sparse_string(struct sparse_ctx * ctx, struct sobj ** obj) {
    struct ostr * str;
    wchar_t escaped_chars[5];
    int escape_pos;
    int escape_state;
    
    str = ostr_new(NULL);

    escape_state = 0;
    /* read everything until " */
    while(1) {
        ctx->prev = ctx->next;
        ctx->next = fgetwc(ctx->in);
        if(ctx->next == WEOF) {
            ostr_destroy(str);
            return SPARSE_EOF;
        }
        
        if(escape_state == 1) {
            /* handle octal char */
            if(ctx->next >= L'0' && ctx->next <= L'7' && escape_pos < 3) {
                escaped_chars[escape_pos] = ctx->next;
                escape_pos++;
            } else {
                escape_state = 0;
                escaped_chars[escape_pos] = L'\0';
                ctx->prev = wcstol(escaped_chars, NULL, 8);
                ostr_replace_last(str, ctx->prev);
                if(ctx->next == L'"') {
                    break;
                } else {
                    ostr_append(str, ctx->next);
                }
            }
        } else if(escape_state == 2) {
            /* handle unicode char */
            /* handle octal char */
            if(iswxdigit(ctx->next) && escape_pos < 4) {
                escaped_chars[escape_pos] = ctx->next;
                escape_pos++;
            } else if (escape_pos == 0) {
                /* bad escape char */
                ostr_destroy(str);
                return SPARSE_BAD_CHAR;
            } else {
                escape_state = 0;
                escaped_chars[escape_pos] = L'\0';
                ctx->prev = wcstol(escaped_chars, NULL, 16);
                ostr_replace_last(str, ctx->prev);
                if(ctx->next == L'"') {
                    break;
                } else {
                    ostr_append(str, ctx->next);
                }
            }
        } else if(ctx->prev == L'\\') {
            /* must replace last entered char with current char */
            if(ctx->next==L'n') {
                ostr_replace_last(str, L'\n');
                ctx->next=L'\n';
            } else if(ctx->next==L'r') {
                ostr_replace_last(str, L'\r');
                ctx->next=L'\r';
            } else if(ctx->next==L'\\' || ctx->next==L'"' || ctx->next==L'\'') {
                ostr_replace_last(str, ctx->next);
            } else if(ctx->next==L'u') {
                escape_pos = 0;
                escape_state = 2;
            } else if(ctx->next >= L'0' && ctx->next <= L'7') {
                escape_pos = 1;
                escape_state = 1;
                escaped_chars[0] = ctx->next;
            } else {
                /* bad escape sequence */
                ostr_destroy(str);
                return SPARSE_BAD_CHAR;
            }
        
        } else if(ctx->next == L'"') {
            break;
        } else {
            ostr_append(str, ctx->next);
        }
    }
        
    *obj = sobj_from_string(ostr_str(str), ostr_length(str));

    ostr_destroy(str);
    return SPARSE_OK;
}

/* not implemented yet... */
int WEAK_FOR_UNIT_TEST
sparse_symbol(struct sparse_ctx * ctx, struct sobj ** obj) {
    return SPARSE_BAD_SYM;
}
int WEAK_FOR_UNIT_TEST
sparse_simple_symbol(struct sparse_ctx * ctx, struct sobj ** obj) {
    return SPARSE_BAD_SYM;
}
int WEAK_FOR_UNIT_TEST
sparse_quote(struct sparse_ctx * ctx, struct sobj ** obj) {
    return SPARSE_BAD_SYM;
}
int WEAK_FOR_UNIT_TEST
sparse_cons(struct sparse_ctx * ctx, struct sobj ** obj) {
    return SPARSE_BAD_SYM;
}
