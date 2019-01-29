#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include "sparser.h"
#include "ostr.h"

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

/* FIXME maybe these should belong to their own file instead 
    of this one alone? */
int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj);
int 
sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj);
int 
sparse_symbol(struct sparse_ctx * ctx, struct sexpression ** obj);
int 
sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj);
int 
sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj);


WEAK_FOR_UNIT_TEST int 
sparse(FILE *in, struct sexpression ** obj) {
    struct sparse_ctx ctx;
    ctx.in = in;
    ctx.next = L' ';
    ctx.stack = NULL;

    sparse_object(&ctx, obj);
    
    /* be careful! it might be WEOF or no read at all! */
    ungetwc(ctx.next, in);

    return 0;
}

WEAK_FOR_UNIT_TEST int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct sexpression * ostr;
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
                /* start or end symbol */
                ret_val = sparse_symbol(ctx, &ostr);
            }
            break;
        }
    }
    
    *obj = ostr;
    
    return ret_val;
}


WEAK_FOR_UNIT_TEST int 
sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct ostr * str;
    wchar_t escaped_chars[5];
    wchar_t *cstr;
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

    cstr=ostr_str(str);
    *obj = sexpr_create_value(cstr, ostr_length(str));
    (*obj)->hint = SC_STRING;
    free(cstr);

    ostr_destroy(str);
    return SPARSE_OK;
}

WEAK_FOR_UNIT_TEST int 
sparse_symbol(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct ostr * str;
    wchar_t * cstr;
    int escape_state;
    int return_value;
    
    str = ostr_new(NULL);

    if(ctx->next == L'#') {
        /* reject symbols starting with # and second char not % */
        escape_state = 3;
    } else {
        escape_state = 0;
    }
    /* read everything until arriving to the end of the symbol */
    while(1) {
        
        if(escape_state == 1) {
            /* handle insides of |....| */
            if (ctx->next == WEOF) {
                /* eof found in a normal symbol. */
                return_value = SPARSE_BAD_SYM;
                goto SYM_PARSE_END;
            } else if(ctx->next == L'|') {
                /* end escape */
                escape_state = 0;
            } else {
                ostr_append(str, ctx->next);
            }
        } else if(escape_state == 2) {
            /* handle insides of \<char> */
            if (ctx->next == WEOF) {
                /* eof found in a normal symbol. */
                return_value = SPARSE_BAD_SYM;
                goto SYM_PARSE_END;
            }
            escape_state = 0;
            ostr_append(str, ctx->next);
        } else if(escape_state == 3) {
            /* I'm not totally sure about this one.... */
            if(ctx->prev == L'#') {
                if(ctx->prev == L'%') {
                    escape_state = 0;
                } else {
                    return_value = SPARSE_BAD_SYM;
                    goto SYM_PARSE_END;
                }
            }
            ostr_append(str, ctx->next);                
        } else {
            if (ctx->next == WEOF) {
                /* eof found in a normal symbol. */
                return_value = SPARSE_OK;
                goto SYM_PARSE_END;
            }
            /* handle "regular" symbol escape */
            switch(ctx->next) {
            case L'|':
                /* start escape context */
                escape_state = 1;
                break;
            
            case L'\\':
                /* regular escape - get next char */
                escape_state = 2;
                break;
            
            case L' ':
            case L'(':
            case L')':
            case L'[': 
            case L']': 
            case L'{': 
            case L'}': 
            case L'"': 
            case L',': 
            case L'\'': 
            case L'`': 
            case L';': 
                /* handle special chars. Put the special char back and finish */
                ungetwc(ctx->next, ctx->in);
                return_value = SPARSE_OK;
                goto SYM_PARSE_END;
                
            default:
                ostr_append(str, ctx->next);
                break;
            }
            
        }
        
        ctx->prev = ctx->next;
        ctx->next = fgetwc(ctx->in);
    }
    
SYM_PARSE_END:
    /* must handle the special case: single dot */
    if(return_value == SPARSE_OK && ostr_length(str) == 1 && ostr_char_at(str, 0) == L'.') {
        return_value = SPARSE_DOT_SYM;
    }
    
    if(return_value == SPARSE_OK) {
        cstr=ostr_str(str);
        *obj = sexpr_create_value(cstr, ostr_length(str));
        free(cstr);
        (*obj)->hint = SC_SYMBOL;
    } else {
        *obj = NULL;
    }

    ostr_destroy(str);
    return return_value;
}

WEAK_FOR_UNIT_TEST int 
sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj) {
    int parse_result;
    struct sexpression * parsed_object;
    struct sexpression * quote;
    
    parse_result = sparse_object(ctx, &parsed_object);
    if(parse_result == SPARSE_OK) {
        /* (quote (parsed_object . NULL)) */
        quote = sexpr_create_value(L"quote", 5);
        quote->hint = SC_SYMBOL;
        *obj = sexpr_cons(quote, sexpr_cons(parsed_object, NULL));
    }
    
    return parse_result;
}

WEAK_FOR_UNIT_TEST int 
sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj) {
    return SPARSE_BAD_SYM;
}

