#include <stdio.h>
#include "test_definitions.h"
#include "assert_sexpr.h"
#include "core_functions.h"
#include "shash.h"
#include "sparser.h"
#include "wcstr_sparser_adapter.h"

static struct sexpression * _sxpr(struct sctx * sctx, wchar_t * str) {
    struct sexpression * sexpr;
    struct sparser_stream * stream;
    stream = create_sparser_stream(WCSTR_ADAPTER, str);
    sparse(stream, &sexpr);
    release_sparser_stream(stream);
    move_to_heap(sctx, sexpr);
    
    return sexpr;
}
static enum sexpression_result 
_mock_lambda(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters);


UnitTest(fn_define_should_return_FN_NULL_SCTX_when_sctx_is_null) {
    struct sexpression * ptr;
    assert_return_code(fn_define(NULL, &ptr, NULL, NULL), FN_NULL_SCTX);
}

UnitTest(fn_define_should_return_FN_NULL_RESULT_when_result_is_null) {
    struct sctx ptr;
    assert_return_code(fn_define(&ptr, NULL, NULL, NULL), FN_NULL_RESULT);
}

// first form (define id expr)

UnitTest(fn_define_should_eval_expression_and_save_the_result_into_global_context) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sexpression * expected_name;
    struct sexpression * expected_value;
    struct sexpression * actual_value;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    arguments = _sxpr(sctx, L"(test \"value\")");
    enter_namespace(sctx);
    
    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_OK);
    
    expected_name = alloc_new_symbol(sctx, L"test", 4);
    expected_value = alloc_new_string(sctx, L"value", 5);
    
    actual_value = (struct sexpression *) shash_search(sctx->global_namespace, expected_name);
    
    assert_sexpr_equal(actual_value, expected_value);
    
    leave_namespace(sctx);

    release_sctx(sctx);
    
}


UnitTest(fn_define_should_raise_error_when_first_argument_is_not_symbol) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    arguments = _sxpr(sctx, L"(\"test\" \"value\")");
    
    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

UnitTest(fn_define_should_raise_error_when_there_no_values) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    arguments = _sxpr(sctx, L"(\"test\")");

    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

UnitTest(fn_define_should_raise_error_when_there_more_than_one_values) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);

    arguments = _sxpr(sctx, L"(test \"value1\" \"value2\")");

    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

// second form (define (head args) body ...+) => translates to (lambda (args) body ...+)
// if(head is list)
UnitTest(fn_define_should_create_implicit_lambda_when_expression_head_is_a_list) {
    struct mem_reference mem_ref;
    struct sexpression * arguments;
    struct sexpression * result;
    struct sexpression * expected_name;
    struct sexpression * expected_body;
    struct sexpression * actual_value;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    /* inject mocked lambda */
    create_global_reference (sctx, L"lambda", 6, &mem_ref);
    *mem_ref.value = alloc_new_function(sctx, _mock_lambda, NULL);
    
    arguments = _sxpr(sctx, L"((fn arg1 arg2) arg2)");

    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_OK);
    
    expected_name = alloc_new_symbol(sctx, L"fn", 2);
    expected_body = _sxpr(sctx, L"((arg1 arg2) arg2)");
    
    actual_value = (struct sexpression *) shash_search(sctx->global_namespace, expected_name);
    
    assert_int_equal(sexpr_type(actual_value), ST_FUNCTION);
    assert_ptr_equal(sexpr_function(actual_value), _mock_lambda);
    assert_sexpr_equal(sexpr_function_closure(actual_value), expected_body);
    
    release_sctx(sctx);
    
}

/* something like (define (fn . args) (apply + args))  =>  (define fn (lambda args (apply + args))) */
UnitTest(fn_define_should_create_implicit_lambda_when_expression_head_is_a_pair) {
    struct mem_reference mem_ref;
    struct sexpression * arguments;
    struct sexpression * result;
    struct sexpression * expected_name;
    struct sexpression * expected_body;
    struct sexpression * actual_value;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    /* inject mocked lambda */
    create_global_reference (sctx, L"lambda", 6, &mem_ref);
    *mem_ref.value = alloc_new_function(sctx, _mock_lambda, NULL);
    
    arguments = _sxpr(sctx, L"((fn . args) (apply + args))");

    assert_return_code(fn_define(sctx, &result, NULL, arguments), FN_OK);
    
    expected_name = alloc_new_symbol(sctx, L"fn", 2);
    expected_body = _sxpr(sctx, L"(args (apply + args))");
    
    actual_value = (struct sexpression *) shash_search(sctx->global_namespace, expected_name);
    
    assert_int_equal(sexpr_type(actual_value), ST_FUNCTION);
    assert_ptr_equal(sexpr_function(actual_value), _mock_lambda);
    assert_sexpr_equal(sexpr_function_closure(actual_value), expected_body);
    
    release_sctx(sctx);
    
}

static enum sexpression_result 
_mock_lambda(struct sctx * sctx, struct sexpression ** result, struct sexpression * closure, struct sexpression * parameters) {
    
    *result = alloc_new_function(sctx, _mock_lambda, parameters);
    
    return FN_OK;
}


