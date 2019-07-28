#include <wchar.h>
#include <locale.h>
#include "sctx.h"
#include "sparser.h"
#include "stdio_sparser_adapter.h"
#include "svisitor.h"
#include "core_functions.h"


static struct sctx * sctx;
static struct sexpression * input_object;
static struct mem_reference answer_ref;

static void interpret_input(void);
static const char* get_sexpression_result_as_str(enum sexpression_result result);

int lsh_main(int argc, char ** argv, char ** envp) {
    struct sparser_stream * stream;
    int return_value;
    struct sctx_config config = {
        .argv = argv,
        .envp = envp,
        .heap_min_size = 128,
        .heap_max_size = 131072,
        .register_static_functions = 1,
        .register_dynamic_functions = 1
    };    
    
    /* configure IO to handle wchar_t correctly */
    
    setlocale(LC_ALL, "");
    
    wprintf(L"S-Expression Interpreter POC\n");
    sctx = create_new_sctx(&config);
    
    create_global_reference(sctx, L"ANS", 3, &answer_ref);
    *answer_ref.value = NULL;
    
    wprintf(L"VM Ready\n> ");
    fflush ( stdout );
    
    stream = create_sparser_stream( FILE_DESCRIPTOR_ADAPTER, stdin);
    
    while((return_value=sparse(stream, &input_object)) != SPARSE_EOF) {
        if(return_value == SPARSE_OK) {
            move_to_heap(sctx, input_object);
            interpret_input();
        } else {
            wprintf(L"Bad input: %d\n", return_value);
        }
        wprintf(L"\n> ");
        fflush ( stdout );
    }
    
    wprintf(L"\nEOF found\n");
    release_sctx(sctx);
    wprintf(L"Bye bye!\n");
    
    return 0;
}

static void interpret_input(void) {
    wprintf(L"\n[DEBUG] %ls\n", L"New object read from stdin");
    dump_sexpr_r(input_object, stdout);
    
    enum sexpression_result result = fn_procedure_step(sctx, answer_ref.value, input_object);
    
    wprintf(L"\n[DEBUG] Eval returned: %s =>\n", get_sexpression_result_as_str(result));
    dump_sexpr_r(*answer_ref.value, stdout);
    
}

#define __FF_SR(symbol) [symbol] = #symbol
const char* _sr_names[] = { __FF_SR(FN_OK), __FF_SR(FN_NULL_SCTX), __FF_SR(FN_NULL_RESULT), __FF_SR(FN_ERROR) };

static const char* get_sexpression_result_as_str(enum sexpression_result result) {
    return _sr_names[result];
}
