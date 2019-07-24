#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "test_definitions.h"
#include "assert_sexpr.h"
#include "core_functions.h"
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

static wchar_t * expected_str;
static size_t expected_str_length;
static size_t expected_str_position;

static wint_t mock_print_char (wchar_t chr) {
    assert_true(expected_str_position < expected_str_length);
    assert_return_code(chr, expected_str[expected_str_position]);
    expected_str_position++;
    return chr;
}

UnitTest(fn_echo_should_return_FN_NULL_SCTX_when_sctx_is_null) {
    struct sexpression * ptr;
    assert_return_code(fn_echo(NULL, &ptr, NULL, NULL), FN_NULL_SCTX);
}

UnitTest(fn_echo_should_return_FN_NULL_RESULT_when_result_is_null) {
    struct sctx ptr;
    assert_return_code(fn_echo(&ptr, NULL, NULL, NULL), FN_NULL_RESULT);
}


// first form (define id expr)

UnitTest(fn_echo_should_write_hello_world) {
    struct sexpression * arguments;
    struct sexpression * result;
    struct sctx * sctx;
    
    sctx = create_new_sctx(NULL);
    arguments = _sxpr(sctx, L"(hello (\"wat\" . \"wat\") \"world\")");
    sctx->print_char = mock_print_char;
    expected_str = L"hello world\n";
    expected_str_length = wcslen(expected_str);
    expected_str_position = 0;
    
    assert_return_code(fn_echo(sctx, &result, NULL, arguments), FN_OK);
    
    assert_int_equal(expected_str_position, expected_str_length);
    
    release_sctx(sctx);
    
}

/* TODO: other types */
