#ifndef __OSTR__H__
#define __OSTR__H__

#include <stdio.h>
#include <wchar.h>

/* what's better? A linked list or realloc ? */

struct ostr_token {
    size_t size;
    size_t length;
    wchar_t *str;
    struct ostr_token *next;
};

struct ostr {
    size_t length;
    struct ostr_token *first;
    struct ostr_token *last;
};


/* function prototypes */

/**
 * Creates a new ostr from a wchar_t string
 */
struct ostr * 
ostr_new(wchar_t *);

/**
 * Destroys an existing ostr.
 */
void 
ostr_destroy(struct ostr *);

/**
 * Append a wchar to an existing ostr
 */
struct ostr * 
ostr_append(struct ostr *, wchar_t);

/**
 * Replace last wchar with given wchar. If str length is zero, returns NULL.
 */
struct ostr * 
ostr_replace_last(struct ostr *, wchar_t);

/**
 * Get the size (length) of an ostr
 */
size_t 
ostr_length(struct ostr *);

/**
 * Concatenate two ostrs
 */
struct ostr * 
ostr_concat(struct ostr *, struct ostr *);

/**
 * Convert a ostr into a NULL terminated string
 */
wchar_t * 
ostr_str(struct ostr *);

/**
 * Compact a multi node ostr into a single node ostr.
 */
struct ostr * 
ostr_compact(struct ostr *);

/**
 * Duplicates a ostr like compact but returns a new ostr instance
 */
struct ostr * 
ostr_dup(struct ostr * ostr);

/**
 * Print a ostr to a FILE stream
 */
int 
ostr_fputs(struct ostr *, FILE*);

/**
 * Print a ostr to stdout
 */
int 
ostr_puts(struct ostr *);

#endif /* __OSTR__H__ */

