#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "test_definitions.h"
#include "assert_sexpr.h"
#include "core_functions.h"
#include "sparser.h"
#include "wcstr_sparser_adapter.h"

static struct sctx * _test_sctx(void);
static struct sexpression * _sxpr(struct sctx * sctx, wchar_t * str);

UnitTest(fn_cond_should_return_FN_NULL_SCTX_when_sctx_is_null) {
    struct sexpression * ptr;
    assert_return_code(fn_cond(NULL, &ptr, NULL, NULL), FN_NULL_SCTX);
}

UnitTest(fn_cond_should_return_FN_NULL_RESULT_when_result_is_null) {
    struct sctx ptr;
    assert_return_code(fn_cond(&ptr, NULL, NULL, NULL), FN_NULL_RESULT);
}


// Assumptions: NIL is false, otherwise true

// (cond ("predicate" "OK")) -> "OK"
UnitTest(fn_cond_should_return_string_OK_when_condition1_is_true) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((\"predicate\" \"OK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}

// (cond (test_sym "OK")) -> "OK"
UnitTest(fn_cond_should_return_string_OK_when_condition2_is_true) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((test_sym \"OK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}

// (cond (test_f "NOK") (test_t "OK")) -> "OK"
UnitTest(fn_cond_should_return_string_OK_when_condition3_is_true) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((test_f \"NOK\") (test_t \"OK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}

// (cond (test_f "NOK1") (test_nil "NOK2") (else "OK")) -> "OK"
UnitTest(fn_cond_should_return_string_OK_when_condition_is_else) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((test_f \"NOK1\") (test_nil \"NOK2\") (else \"OK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}


// (cond (test_f "NOK1") (else "OK") (test_t "NOK2")) -> "OK"
UnitTest(fn_cond_should_not_process_predicates_when_condition_is_else_is_found) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((test_f \"NOK1\") (else \"OK\") (test_t \"NOK2\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}

// (cond (test_f "NOK1") (test_nil "NOK2")) -> NIL
UnitTest(fn_cond_should_return_null_value_when_no_condition_is_true) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((test_f \"NOK1\") (test_nil \"NOK2\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_ptr_equal(result, NULL);
    
    release_sctx(sctx);
}

// (cond ("t" "NOK1" "NOK2" "OK")) -> "OK"
UnitTest(fn_cond_should_evaluate_all_elements_and_return_last_one_when_predicate_is_true) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((\"true\" \"NOK1\" \"NOK2\" \"OK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_OK);
    
    assert_sexpr_equal(result, alloc_new_string(sctx, L"OK", 2));
    
    release_sctx(sctx);
}

// (cond ("t" "OK") "NOK) -> ERROR
UnitTest(fn_cond_should_return_FN_ERROR_when_any_argument_is_not_list) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((\"true\" \"OK\") \"NOK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_ERROR);
    
    release_sctx(sctx);
    
}

// (cond ("t" "OK") ("NOK)) -> ERROR
UnitTest(fn_cond_should_return_FN_ERROR_when_any_argument_is_list_with_less_than_2_elements) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx = _test_sctx();
    
    arguments = _sxpr(sctx, L"((\"true\" \"OK\") (\"NOK\"))");
    
    assert_return_code(fn_cond(sctx, &result, NULL, arguments), FN_ERROR);
    
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

static struct sctx * _test_sctx(void) {
    struct mem_reference reference;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL);
    
    create_global_reference(sctx, L"test_sym", 8, &reference);
    *reference.value = alloc_new_string(sctx, L"TRUE", 4);
    
    create_global_reference(sctx, L"test_nil", 8, &reference);
    *reference.value = NULL;
    
    create_global_reference(sctx, L"test_t", 6, &reference);
    *reference.value = alloc_new_string(sctx, L"#t", 2);
    
    create_global_reference(sctx, L"test_f", 6, &reference);
    *reference.value = alloc_new_symbol(sctx, L"#f", 2);
    
    
    return sctx;
}
    

