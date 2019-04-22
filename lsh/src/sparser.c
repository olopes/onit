#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include "sparser.h"
#include "sparser_privates.h"
#include "ostr.h"


/* sintatic suggar */
#define READ_CHAR(stream) (stream)->adapter->read_char((stream))

#define UNREAD_CHAR(stream, chr) (stream)->adapter->unread_char((stream), (chr))


/*
From Racket docs:
Any string (i.e., any character sequence) can be supplied to string->symbol to obtain the corresponding symbol. For reader input, any character can appear directly in an identifier, except for whitespace and the following special characters:

   ( ) [ ] { } " , ' ` ; # | \

Actually, # is disallowed only at the beginning of a symbol, and then only if not followed by %; otherwise, # is allowed, too. Also, . by itself is not a symbol.

Whitespace or special characters can be included in an identifier by quoting them with | or \. These quoting mechanisms are used in the printed form of identifiers that contain special characters or that might otherwise look like numbers.

; is a comment
*/





WEAK_FOR_UNIT_TEST int 
sparse(struct sparser_stream * stream, struct sexpression ** obj) {
    struct sparse_ctx ctx;
    ctx.stream = stream;
    ctx.next = L' ';
    ctx.prev = L' ';

    return sparse_object(&ctx, obj);
    
}


static int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct sexpression * sexpr;
    int ret_val;
    ret_val = SPARSE_OK;
    sexpr = NULL;
    
    while(1) {
        ctx->prev = ctx->next;
        ctx->next = READ_CHAR(ctx->stream);
        if(ctx->next == WEOF) {
            ret_val = SPARSE_EOF;
            break;
        }

        if(iswspace(ctx->next)) {
            continue;
        } else {

            if(ctx->next == L'"') {
                // start or end str
                ret_val = sparse_string(ctx, &sexpr);
            } else if(ctx->next == L'(') {
                /* start cons */
                ret_val = sparse_cons(ctx, &sexpr);
            } else if(ctx->next == L')') {
                /* start cons */
                ret_val = SPARSE_PAREN;
            } else if(ctx->next == L'\'') {
                /* 
                start quote - the quote ends with the current expression. 
                translates to (quote xxxx)
                */
                ret_val = sparse_quote(ctx, &sexpr);
            } else if(ctx->next == L';') {
                /* start single line comment */
                sexpr = sexpr_create_value(L"\n",1);
                ret_val = SPARSE_COMMENT;
            } else {
                /* start or end symbol */
                ret_val = sparse_symbol(ctx, &sexpr);
            }
            
            /* check if we stumbled into a comment */
            if(ret_val == SPARSE_COMMENT) {
                ret_val = sparse_comment(ctx, sexpr);
                sexpr_free_object(sexpr);
                sexpr = NULL;
                if(ret_val == SPARSE_OK) {
                    continue;
                }
            }
            break;
        }
    }
    
    *obj = sexpr;
    
    return ret_val;
}


static int 
sparse_string(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct ostr * str;
    wchar_t escaped_chars[5];
    wchar_t *cstr;
    int escape_pos;
    int escape_state;
    
    str = ostr_new(NULL);

    escape_state = 0;
    escape_pos = 0;
    /* read everything until " */
    while(1) {
        ctx->prev = ctx->next;
        ctx->next = READ_CHAR(ctx->stream);
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
        } else if(escape_state == 3) {
            ostr_append(str, ctx->next);
            escape_state = 0;
        } else if(ctx->prev == L'\\') {
            /* must replace last entered char with current char */
            if(ctx->next == L'n') {
                ostr_replace_last(str, L'\n');
                ctx->next = L'\n';
            } else if(ctx->next == L'r') {
                ostr_replace_last(str, L'\r');
                ctx->next=L'\r';
            } else if(ctx->next == L'"' || ctx->next == L'\'') {
                ostr_replace_last(str, ctx->next);
            } else if(ctx->next == L'u') {
                escape_pos = 0;
                escape_state = 2;
            } else if(ctx->next >= L'0' && ctx->next <= L'7') {
                escape_pos = 1;
                escape_state = 1;
                escaped_chars[0] = ctx->next;
            } else if(ctx->next == L'\\') {
                escape_state = 3;
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
    (*obj)->content = SC_STRING;
    free(cstr);

    ostr_destroy(str);
    return SPARSE_OK;
}

static int 
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
                if(ctx->next == L'%') {
                    escape_state = 0;
                } else if(ctx->next == L'=') {
                    /* append to comment body */
                } else if(ctx->next == L'|') {
                    /* in a comment */
                    ostr_append(str, L'|');
                    return_value = SPARSE_COMMENT;
                    goto SYM_PARSE_END;
                } else {
                    return_value = SPARSE_BAD_SYM;
                    goto SYM_PARSE_END;
                }
            } else if(ctx->prev == L'=') {
                if(ctx->next == L'=') {
                    /* append to comment body */
                } else if(ctx->next == L'|') {
                    /* in a comment */
                    ostr_append(str, L'|');
                    return_value = SPARSE_COMMENT;
                    goto SYM_PARSE_END;
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
            if(iswspace(ctx->next)) {
                /* handle special chars. Put the special char back and finish */
                UNREAD_CHAR(ctx->stream, ctx->next);
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
                UNREAD_CHAR(ctx->stream, ctx->next);
                return_value = SPARSE_OK;
                goto SYM_PARSE_END;
                
            default:
                ostr_append(str, ctx->next);
                break;
            }
            
        }
        
        ctx->prev = ctx->next;
        ctx->next = READ_CHAR(ctx->stream);
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
        (*obj)->content = SC_SYMBOL;
    } else if(return_value == SPARSE_COMMENT) {
        cstr=ostr_str(str);
        cstr[0]=L'|';
        cstr[ostr_length(str)-1] = L'#';
        *obj = sexpr_create_value(cstr, ostr_length(str));
        free(cstr);
        (*obj)->content = SC_SYMBOL;
    } else {
        *obj = NULL;
    }

    ostr_destroy(str);
    return return_value;
}

static int 
sparse_quote(struct sparse_ctx * ctx, struct sexpression ** obj) {
    int parse_result;
    struct sexpression * parsed_object;
    struct sexpression * quote;
    
    parse_result = sparse_object(ctx, &parsed_object);
    if(parse_result == SPARSE_OK) {
        /* (quote (parsed_object . NULL)) */
        quote = sexpr_create_value(L"quote", 5);
        quote->content = SC_SYMBOL;
        *obj = sexpr_cons(quote, sexpr_cons(parsed_object, NULL));
    }
    
    return parse_result;
}

static int 
sparse_cons(struct sparse_ctx * ctx, struct sexpression ** obj) {
    struct sexpression * sexpr;
    struct sexpression * list;
    int sparse_object_result;
    int return_value;
    int state;
    
    list = NULL;
    state = 0;

    /* FIXME here be memleaks */
    
    while(1) {
        sparse_object_result = sparse_object(ctx, &sexpr);
        
        if(sparse_object_result == SPARSE_EOF) {
            goto EXIT_WITH_ERROR;
        } else if(sparse_object_result == SPARSE_PAREN) {
            return_value = SPARSE_OK;
            if(state == 1) {
                *obj = list;
            } else {
                *obj = sexpr_reverse(list);
            }
            goto EXIT_WITH_SUCCESS;
        } else if(state == 1 && sparse_object_result != SPARSE_OK) {
            goto EXIT_WITH_ERROR;
        } else if(sparse_object_result == SPARSE_DOT_SYM && list == NULL) {
            goto EXIT_WITH_ERROR;
        } else if(sparse_object_result == SPARSE_DOT_SYM) {
            state = 1;
        } else if(state == 1 && sparse_object_result == SPARSE_OK) {
            if(list->cdr.sexpr != NULL) {
                goto EXIT_WITH_ERROR;
            } else {
                list->cdr.sexpr = sexpr;
            }
        } else if(sparse_object_result == SPARSE_OK) {
            list = sexpr_cons(sexpr, list);
        } else {
            return_value = sparse_object_result;
            *obj = NULL;
            goto EXIT_WITH_SUCCESS;
        }
        
    }

EXIT_WITH_ERROR:
    sexpr_free(list);
    return_value = SPARSE_BAD_SYM;
    *obj = NULL;
    
EXIT_WITH_SUCCESS:
    return return_value;
}

static int 
sparse_comment(struct sparse_ctx * ctx, struct sexpression * sexpr_eoc) {
    int return_value;
    wchar_t * end_of_comment;
    size_t comment_len;
    size_t pos;
    
    comment_len = sexpr_length(sexpr_eoc);
    end_of_comment = sexpr_value(sexpr_eoc);
    pos = 0;
    
    while(1) {
        if (ctx->next == WEOF) {
            /* eof found in a normal symbol. */
            return_value = SPARSE_EOF;
            break;
        } else if(ctx->next == end_of_comment[pos]) {
            pos++;
            if(pos == comment_len) {
                return_value = SPARSE_OK;
                break;
            }
        } else {
            pos = 0;
        }

        ctx->prev = ctx->next;
        ctx->next = READ_CHAR(ctx->stream);
    }
    return return_value;
}
