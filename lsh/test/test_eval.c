#include "test_definitions.h"
#include "assert_sexpr.h"
#include "eval.h"

UnitTest(eval_should_return_null_when_either_parameter_is_null) {
    struct sexpression dummy_sexpr;
    struct sctx dummy_sctx;
    
    
    assert_null(eval_sexpr(NULL, &dummy_sexpr));
    
    assert_null(eval_sexpr(&dummy_sctx, NULL));
    
}

UnitTest(eval_should_return_parameter_sexpression_when_parameter_sexpression_is_string) {
    struct sexpression dummy_sexpr;
    struct sctx dummy_sctx;
    
    dummy_sexpr.type=ST_STRING;
    
    
    assert_ptr_equal(eval_sexpr(&dummy_sctx, &dummy_sexpr), &dummy_sexpr);
    
}

UnitTest(eval_should_return_referenced_value_in_sctx_when_parameter_is_a_symbol) {
    struct mem_reference ref1;
    struct sexpression * symbol;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"ref", 3, &ref1);
    *ref1.value = alloc_new_string(sctx, L"val", 3);
   
    symbol = alloc_new_symbol(sctx, L"ref", 3);
    
    assert_ptr_equal(eval_sexpr(sctx, symbol), *ref1.value);

    
    release_sctx(sctx);
}


static struct sexpression * test_function(struct sctx * sctx, struct sexpression * sexpr) {
    assert_sexpr_equal(sexpr_car(sexpr), alloc_new_string(sctx, L"PARAM", 5));
    return alloc_new_string(sctx, L"PASS", 4);
}


UnitTest(eval_should_call_function_when_parameter_is_a_list_and_head_references_a_function) {
    struct mem_reference ref1;
    struct sexpression * expr;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"fn", 2, &ref1);
    *ref1.value = alloc_new_function(sctx, test_function);
   
    expr = alloc_new_pair(sctx, 
                          alloc_new_symbol(sctx, L"fn", 2), 
                          alloc_new_pair(sctx, alloc_new_string(sctx, L"PARAM", 5), NULL));
    
    
    assert_sexpr_equal(eval_sexpr(sctx, expr), alloc_new_string(sctx, L"PASS", 4));

    
    release_sctx(sctx);
}


UnitTest(eval_should_raise_error_when_parameter_is_a_list_and_head_does_not_references_a_function, .ignore=1) {
    struct mem_reference ref1;
    struct sexpression * symbol;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"ref", 3, &ref1);
    *ref1.value = alloc_new_string(sctx, L"val", 3);
   
    symbol = alloc_new_symbol(sctx, L"ref", 3);
    
    assert_ptr_equal(eval_sexpr(sctx, symbol), *ref1.value);

    
    release_sctx(sctx);
}

