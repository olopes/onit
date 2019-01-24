#ifndef __SPARSER_H__
#define __SPARSER_H__

#include <stdio.h>
#include "stype.h"

#define SPARSE_OK 0
#define SPARSE_EOF 1
#define SPARSE_BAD_CHAR 2
#define SPARSE_PAREN 3
#define SPARSE_BAD_SYM 4
#define SPARSE_DOT_SYM 5


/**
 * Read (parse) a S-Expression object from the specified stream 
 */
int
sparse(FILE * input, struct sobj ** parsed_object);


/**
 * Release a sparsed S-Expression Object created by sparse function
 */
void
sparse_free(struct sobj *);

#endif /* __SPARSER_H__ */
