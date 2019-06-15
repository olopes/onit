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

static sexpression_callable fn_lambda_closure;

BeforeAll(extract_lambda_procedure) {
    struct sexpression * result;
    struct sctx * sctx;
    sctx = create_new_sctx(NULL, NULL);
    fn_lambda(sctx, &result, NULL, _sxpr(sctx, L"(() \"X\")"));
    fn_lambda_closure = sexpr_function(result);
    release_sctx(sctx);

    return 0;
}

UnitTest(fn_lambda_should_return_FN_NULL_SCTX_when_sctx_is_null) {
    struct sexpression * ptr;
    assert_return_code(fn_lambda(NULL, &ptr, NULL, NULL), FN_NULL_SCTX);
}

UnitTest(fn_lambda_should_return_FN_NULL_RESULT_when_result_is_null) {
    struct sctx ptr;
    assert_return_code(fn_lambda(&ptr, NULL, NULL, NULL), FN_NULL_RESULT);
}

UnitTest(fn_lambda_should_return_FN_ERROR_when_parameters_list_is_null) {
    struct sctx ptr;
    struct sexpression * result;
    assert_return_code(fn_lambda(&ptr, &result, NULL, NULL), FN_ERROR);
}

UnitTest(fn_lambda_should_return_FN_ERROR_when_parameters_is_not_pair) {
    struct sctx ptr;
    struct sexpression * result;
    struct sexpression * argument = sexpr_create_string(L"AA", 2);
    assert_return_code(fn_lambda(&ptr, &result, NULL, argument), FN_ERROR);
    sexpr_free(argument);
}

// first form (lambda (arg1 arg2) expr1 expr2)

UnitTest(fn_lambda_should_return_a_procedural_closure_when_first_argument_is_pair) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    arguments = _sxpr(sctx, L"((arg1 arg2) expr1 expr2)");

    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_OK);
    
    assert_ptr_equal(sexpr_function_closure(result), arguments);
    
    release_sctx(sctx);
    
}

UnitTest(fn_lambda_should_return_a_procedural_closure_when_first_argument_is_symbol) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    /* arguments stored in the symbol 'args' */
    arguments = _sxpr(sctx, L"(args expr1 expr2)");

    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_OK);
    
    assert_ptr_equal(sexpr_function_closure(result), arguments);
    
    release_sctx(sctx);
    
}

UnitTest(fn_lambda_should_return_a_procedural_closure_when_first_argument_is_nil) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    /* no arguments lambda */
    arguments = _sxpr(sctx, L"(() expr1 expr2)");

    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_OK);
    
    assert_ptr_equal(sexpr_function_closure(result), arguments);
    
    release_sctx(sctx);
    
}


UnitTest(fn_lambda_should_raise_error_when_first_argument_is_not_nil_or_symbol_or_pair) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    /* string */
    arguments = _sxpr(sctx, L"(\"test\" expr1 expr2)");
    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

UnitTest(fn_lambda_should_return_a_procedural_closure_when_first_argument_is_list_and_have_symbol_or_nil_elements) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    /* capture 1st, 2nd, 4th arguments in specific vars and a list with the remaining */
    arguments = _sxpr(sctx, L"((arg1 arg2 () arg4 . remaining) expr1 expr2)");
    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_OK);
    
    assert_ptr_equal(sexpr_function_closure(result), arguments);
    
    release_sctx(sctx);
    
}

UnitTest(fn_lambda_should_raise_error_when_first_argument_is_list_and_have_elements_other_than_symbol_or_nil) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL, NULL);
    
    /* string */
    arguments = _sxpr(sctx, L"((arg1 arg2 () arg3 \"test\") expr1 expr2)");
    assert_return_code(fn_lambda(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

/* TODO compile references? */


/* TODO implement procedure execution.
 * Assertions on the procedure:
 * - create namespace
 * - bind arguments
 * - eval expressions in order
 * - leave namespace
 * - return last expression evaluation result
 * 
 */
