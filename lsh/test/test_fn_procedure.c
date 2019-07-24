#include <stdio.h>
#include "test_definitions.h"
#include "assert_sexpr.h"
#include "core_functions.h"
#include "eval.h"
#include "sparser.h"
#include "wcstr_sparser_adapter.h"

static struct sexpression * _sxpr(struct sctx * sctx, wchar_t * str);
static struct sexpression * 
register_function(struct sctx * sctx, wchar_t * name, sexpression_callable function);
static struct sexpression * 
register_string_value (struct sctx * sctx, wchar_t * name, wchar_t * value);

static enum sexpression_result
function1(struct sctx *, struct sexpression **, struct sexpression *, struct sexpression *);
static enum sexpression_result
function2(struct sctx *, struct sexpression **, struct sexpression *, struct sexpression *);
static enum sexpression_result
function3(struct sctx *, struct sexpression **, struct sexpression *, struct sexpression *);

static int function1_called;
static int function2_called;
static int function3_called;

UnitTest(fn_procedure_should_return_FN_NULL_SCTX_when_sctx_is_null) {
    struct sexpression * ptr;
    assert_return_code(fn_lambda(NULL, &ptr, NULL, NULL), FN_NULL_SCTX);
}

UnitTest(fn_procedure_should_return_FN_NULL_RESULT_when_result_is_null) {
    struct sctx ptr;
    assert_return_code(fn_lambda(&ptr, NULL, NULL, NULL), FN_NULL_RESULT);
}

UnitTest(fn_procedure_should_return_FN_error_when_parameters_is_null) {
    struct sctx ptr;
    struct sexpression * result;
    assert_return_code(fn_lambda(&ptr, &result, NULL, NULL), FN_ERROR);
}



UnitTest(fn_procedure_should_execute_parameters_in_order_and_return_last_result) {
    struct sexpression * expression;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL);
    
    register_function(sctx, L"lambda", fn_lambda);
    register_function(sctx, L"func1", function1);
    register_function(sctx, L"func2", function2);
    register_function(sctx, L"func3", function3);
    register_string_value(sctx, L"str", L"text");
    register_string_value(sctx, L"arg", L"456");
    
    expression = _sxpr(sctx, L"((func1 arg) \"123\" (func2) (func3 arg str))");
    
    /* act */
    assert_return_code(fn_procedure(sctx, &result, NULL, expression), FN_OK);

    /* assert results - no substitution is performed */
    assert_sexpr_equal(result, _sxpr(sctx, L"(arg str)"));
    
    assert_true(function1_called);
    assert_true(function2_called);
    assert_true(function3_called);
    
    release_sctx(sctx);
    
}

static struct sexpression * _sxpr(struct sctx * sctx, wchar_t * str) {
    struct sexpression * sexpr;
    struct sparser_stream * stream;
    stream = create_sparser_stream(WCSTR_ADAPTER, str);
    sparse(stream, &sexpr);
    release_sparser_stream(stream);
    move_to_heap(sctx, sexpr);
    
    return sexpr;
}

static struct sexpression *
register_function(struct sctx * sctx, wchar_t * name, sexpression_callable function) {
    struct mem_reference reference;
    create_global_reference(sctx, name, wcslen(name), &reference);
    return  *reference.value = alloc_new_function(sctx, function, NULL);
}

static struct sexpression *
register_string_value (struct sctx * sctx, wchar_t * name, wchar_t * value) {
    struct mem_reference reference;
    create_global_reference(sctx, name, wcslen(name), &reference);
    return *reference.value = alloc_new_string(sctx, value, wcslen(value));
}



static enum sexpression_result
function1(
    struct sctx * sctx, 
    struct sexpression ** result,
    struct sexpression * closure, 
    struct sexpression * parameters) {
    (void)sctx;
    (void)closure;
    
    *result = parameters;
    function1_called = 1;
    
    return FN_OK;
}

static enum sexpression_result
function2(
    struct sctx * sctx, 
    struct sexpression ** result,
    struct sexpression * closure, 
    struct sexpression * parameters) {
    (void)sctx;
    (void)closure;
    
    *result = parameters;
    function2_called = 1;
    
    return FN_OK;
}

static enum sexpression_result
function3(
    struct sctx * sctx, 
    struct sexpression ** result,
    struct sexpression * closure, 
    struct sexpression * parameters) {
    (void)sctx;
    (void)closure;
    
    *result = parameters;
    function3_called = 1;
    
    return FN_OK;
}

