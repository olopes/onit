#ifndef __SCTX_H__
#define __SCTX_H__


typedef struct sexpression * (*primitive_fn)(void * sctx, struct sexpression  *);

extern void * 
init_environment(char **argv, char **envp);

extern int 
register_primitive_fn(void * sctx, struct svalue * name, primitive_fn * fn);

extern int 
register_primitive_obj(void * sctx, struct svalue * name, void * obj);

extern int 
register_primitive_value(void * sctx, struct svalue * name, struct sexpression * fn);

extern int 
enter_namespace(void * sctx);

extern int 
leave_namespace(void * sctx);

extern struct sexpression * 
lookup_name(void * sctx, struct svalue * name);

extern int 
register_value(void * sctx, struct svalue * name, struct sexpression * value);

#endif
