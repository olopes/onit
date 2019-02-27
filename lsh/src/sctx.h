#ifndef __SCTX_H__
#define __SCTX_H__


typedef struct sexpression * (*primitive_fn)(void * sctx, struct sexpression  *);

extern void * 
init_environment(char **argv, char **envp);

extern int 
register_primitive_fn(void * sctx, struct svalue * name, primitive_fn * fn);

extern int 
register_primitive_object(void * sctx, struct svalue * name, void * obj);

extern struct sexpression * 
alloc_new_pair(void * sctx, struct sexpression * car, struct sexpression * cdr);

extern struct sexpression * 
alloc_new_value(void * sctx, wchar_t * wcstr, size_t len);

extern int 
enter_namespace(void * sctx);

extern int 
leave_namespace(void * sctx);

extern struct sexpression * 
lookup_name(void * sctx, struct svalue * name);

extern int 
register_value(void * sctx, struct svalue * name, struct sexpression * value);

#endif
