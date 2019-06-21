#include "test_definitions.h"
#include "assert_sexpr.h"
#include "eval.h"

static enum sexpression_result 
test_function(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * args);

UnitTest(eval_should_return_null_when_either_parameter_is_null) {
    struct sexpression * dummy_sexpr;
    struct sctx dummy_sctx;
    
    
    assert_return_code(eval_sexpr(NULL, &dummy_sexpr, NULL), FN_NULL_SCTX);
    
    assert_return_code(eval_sexpr(&dummy_sctx, NULL, NULL), FN_NULL_RESULT);
    
}

UnitTest(eval_should_return_parameter_sexpression_when_parameter_sexpression_is_string) {
    struct sexpression dummy_sexpr;
    struct sexpression * result;
    struct sctx * sctx = create_new_sctx(NULL, NULL);
    
    dummy_sexpr.type=ST_STRING;
    
    assert_return_code(eval_sexpr(sctx, &result, &dummy_sexpr), FN_OK);
    
    assert_ptr_equal(result, &dummy_sexpr);
    
    release_sctx(sctx);
}

UnitTest(eval_should_return_referenced_value_in_sctx_when_parameter_is_a_symbol) {
    struct mem_reference ref1;
    struct sexpression * symbol;
    struct sexpression * result;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"ref", 3, &ref1);
    *ref1.value = alloc_new_string(sctx, L"val", 3);
   
    symbol = alloc_new_symbol(sctx, L"ref", 3);
    
    assert_return_code(eval_sexpr(sctx, &result, symbol), FN_OK);
    
    assert_ptr_equal(result, *ref1.value);

    
    release_sctx(sctx);
}


UnitTest(eval_should_call_function_when_parameter_is_a_list_and_head_references_a_function) {
    struct mem_reference ref1;
    struct sexpression * expr;
    struct sexpression * result;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"fn", 2, &ref1);
    *ref1.value = alloc_new_function(sctx, test_function, NULL);
   
    expr = alloc_new_pair(sctx, 
                          alloc_new_symbol(sctx, L"fn", 2), 
                          alloc_new_pair(sctx, alloc_new_string(sctx, L"PARAM", 5), NULL));
    
    
    assert_return_code(eval_sexpr(sctx, &result, expr), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"PASS", 4));

    
    release_sctx(sctx);
}

static enum sexpression_result 
test_function(struct sctx * sctx, struct sexpression ** result, struct sexpression * body, struct sexpression * args) {
    assert_sexpr_equal(sexpr_car(args), alloc_new_string(sctx, L"PARAM", 5));
    *result = alloc_new_string(sctx, L"PASS", 4);
    return FN_OK;
}



UnitTest(eval_should_raise_error_when_parameter_is_a_list_and_head_does_not_references_a_function) {
    struct mem_reference ref1;
    struct sexpression * expr;
    struct sexpression * result;
    struct sctx * sctx = create_new_sctx(NULL, NULL);

    create_global_reference(sctx, L"ref", 3, &ref1);
    *ref1.value = alloc_new_string(sctx, L"val", 3);
   
    expr = alloc_new_pair(sctx, 
                          alloc_new_symbol(sctx, L"ref", 3), 
                          alloc_new_pair(sctx, alloc_new_string(sctx, L"PARAM", 5), NULL));
    
    assert_return_code(eval_sexpr(sctx, &result, expr), FN_ERROR);

    assert_true(sexpr_is_error(result));
    
    release_sctx(sctx);
}
