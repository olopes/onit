#ifdef _INCLUDE_SCTX_C_

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

static void load_array_to_sexpr(struct sexpression ** list, char ** array);
static void load_primitives(struct sctx * sctx);
static wchar_t * convert_to_wcstr(const char * src);
static void recicle(struct sctx * sctx);


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
    
    heap = (struct sexpression **) malloc(sizeof(struct sexpression *)*MIN_OBJ);
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
        .heap_size = MIN_OBJ,
        .heap_load = 0,
        .heap = heap,
    };
    
    
    /* setup locale? or assume already configured? */
    
    load_array_to_sexpr(&sctx->arguments, argv);
    load_array_to_sexpr(&sctx->environment, envp);
    
    load_primitives(sctx);
    
    /* create two references */
}



static void load_array_to_sexpr(struct sexpression ** list_ptr, char ** array) {
    wchar_t * wcstr;
    struct sexpression * list;
    list = NULL;
    
    while(*array) {
        wcstr = convert_to_wcstr(*array);        
        list = sexpr_cons(sexpr_create_value(wcstr, wcslen(wcstr)), list);
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
    
    
}



int register_primitive_fn(void * sctx, struct svalue * name, primitive_fn * fn) {
    
}

int register_primitive_obj(void * sctx, struct svalue * name, void * obj) {
    
}

int register_primitive_value(void * sctx, struct svalue * name, struct sexpression * fn) {
    
}


#endif
