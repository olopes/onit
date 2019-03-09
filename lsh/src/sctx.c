#include "sctx_privates.h"

static void load_array_to_sexpr(struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void destroy_primitive_references(void * sctx, struct svalue * name, void * primitive_ptr);

/* some static/constant names */
static struct svalue key_true = {
    .len = 2,
    .data = L"#t",
};

static struct svalue key_false = {
    .len = 2,
    .data = L"#f",
};

static struct svalue key_args = {
    .len = 5,
    .data = L"#args",
};

static struct svalue key_env = {
    .len = 4,
    .data = L"#env",
};

static struct primitive TRUE_PRIMITIVE = {
    .type = PRIMITIVE_VALUE,
    .value = { .value = &key_true },
    .destructor = NULL,
};

static struct primitive FALSE_PRIMITIVE = {
    .type = PRIMITIVE_VALUE,
    .value = { NULL },
    .destructor = NULL,
};

static struct primitive ARGUMENTS_PRIMITIVE = {
    .type = PRIMITIVE_SEXPRESSION,
    .value = { NULL },
    .destructor = NULL,
};
static struct primitive ENVIRONMENT_PRIMITIVE = {
    .type = PRIMITIVE_SEXPRESSION,
    .value = { NULL },
    .destructor = NULL,
};


void * 
init_environment(char **argv, char **envp) {
    struct sctx * sctx;
    struct sexpression ** heap;
    struct sexpression * arg_list;
    struct sexpression * env_list;
    
    
    sctx = (struct sctx *) malloc(sizeof(struct sctx));
    if(sctx == NULL) {
        return NULL;
    }
    
    heap = (struct sexpression **) malloc(sizeof(struct sexpression *) * HEAP_MIN_SIZE);
    if(heap == NULL) {
        free(sctx);
        return NULL;
    }
    
    *sctx = (struct sctx) {
        .primitives = {
            .size=0,
            .load=0,
            .table=NULL,
        },
        .namespaces = NULL,
        .environment = NULL,
        .arguments = NULL,
        .visit = 0,
        .heap_size = HEAP_MIN_SIZE,
        .heap_load = 0,
        .heap = heap,
        .namespace_destructor = NULL,
    };
    
    
    /* setup locale? or assume already configured? */
    
    load_array_to_sexpr(&sctx->arguments, argv);
    load_array_to_sexpr(&sctx->environment, envp);
    
    load_primitives(sctx);
    
    /* create two references */
    enter_namespace(sctx);
    /* register args and env. forget about primitives*/

    
    return sctx;
}



static void load_array_to_sexpr(struct sexpression ** list_ptr, char ** array) {
    wchar_t * wcstr;
    struct sexpression * list;
    list = NULL;
    
    while(*array) {
        wcstr = convert_to_wcstr(*array);        
        list = sexpr_cons(sexpr_create_value(wcstr, wcslen(wcstr)), list);
        free(wcstr);
        array++;
    }
    
    *list_ptr = sexpr_reverse(list);
    
}    

static wchar_t * convert_to_wcstr(const char * src) {
    wchar_t * wcstr;
    size_t len;
    size_t conv;
    
    len = mbstowcs(NULL, src, 0);
    if(len == -1) {
        return NULL;
    }
    wcstr = (wchar_t *) malloc(sizeof(wchar_t)*(len+1));
    if(wcstr == NULL) {
        return NULL;
    }
    conv = mbstowcs(wcstr, src, len+1);
    if(conv != len) {
        free(wcstr);
        return NULL;
    }
    return wcstr;
}


static void load_primitives(struct sctx * sctx) {
    
    register_primitive(sctx, &key_true, &TRUE_PRIMITIVE);
    register_primitive(sctx, &key_false, &FALSE_PRIMITIVE);
    
    /*
    ARGUMENTS_PRIMITIVE.value.sexpression = sctx->arguments;
    register_primitive(sctx, &key_args, &ARGUMENTS_PRIMITIVE);
    
    ENVIRONMENT_PRIMITIVE.value.sexpression = sctx->environment;
    register_primitive(sctx, &key_env, &ENVIRONMENT_PRIMITIVE);
    */
}

int register_primitive(void * sctx_ptr, struct svalue * name, struct primitive * primitive) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(sctx == NULL || name == NULL) {
        return 1;
    }
    
    if(shash_has_key(&sctx->primitives, name)) {
        return 2;
    }
    
    return shash_insert(&sctx->primitives, name, primitive);
}



void 
release_environment(void * sctx_ptr) {
    struct sctx * sctx = (struct sctx *) sctx_ptr;
    
    if(sctx == NULL) {
        return;
    }
    
    sexpr_free(sctx->arguments);
    sexpr_free(sctx->environment);
    
    /* TODO release remaining primitives before releasing the hashtable */
    shash_visit(&sctx->primitives, sctx, destroy_primitive_references);
    shash_free(&sctx->primitives);

    leave_namespace(sctx);
    sctx_gc(sctx);
    
    free(sctx->heap);
    free(sctx);
}

static void destroy_primitive_references(void * sctx, struct svalue * name, void * primitive_ptr) {
    struct primitive * primitive = (struct primitive *) primitive_ptr;
    
    if(primitive != NULL && primitive->destructor != NULL) {
        primitive->destructor(sctx, primitive);
    }
}

