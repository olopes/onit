/* prod code includes */
#include "test_definitions.h"
#include "sexpr.h"

#include "assert_sexpr.h"

UnitTest(sexpr_cons_should_alloc_new_sexpression)
{
    struct sexpression * sexpr;
    struct sexpression dummy_car;
    struct sexpression dummy_cdr;
    
    sexpr = sexpr_cons(&dummy_car, &dummy_cdr);
    
    assert_non_null(sexpr);
    assert_ptr_equal(sexpr->data.sexpr, &dummy_car);
    assert_ptr_equal(sexpr->cdr.sexpr, &dummy_cdr);
    assert_int_equal(sexpr->type, ST_CONS);
    
    free(sexpr);
}

UnitTest(sexpr_create_value_should_alloc_new_sexpression_and_alloc_value_string)
{
    struct sexpression * sexpr;
    
    sexpr = sexpr_create_symbol(L"TEST", 4);
    
    assert_non_null(sexpr);
    assert_int_equal(sexpr->type, ST_SYMBOL);
    assert_int_equal(sexpr->len, 4);
    assert_true(wcsncmp(L"TEST", sexpr->data.value, 4) == 0);
    
    free(sexpr->data.value);
    free(sexpr);
}

UnitTest(sexpr_free_value_should_release_sexpression_recursively)
{
    struct sexpression * sexpr;
    
    /* alloc list (CAR CDR) */
    sexpr = sexpr_cons(
        sexpr_create_symbol(L"CAR", 3),
        sexpr_cons(
            sexpr_create_string(L"CDR", 3), NULL
        )
    );
    
    sexpr_free(sexpr);

    /* cmocka will validate allocated/released chunks */
}

UnitTest(sexpr_free_object_value_should_release_only_the_first_pair)
{
    struct sexpression * sexpr1;
    struct sexpression * sexpr2;
    struct sexpression * sexpr3;
    
    /* alloc list (CAR CDR) */
    sexpr1 = sexpr_cons(
        sexpr2 = sexpr_create_string(L"CAR", 3),
        sexpr3 = sexpr_create_string(L"CDR", 3)
    );
    
    /* only sexpr_cons pair will be freed */
    sexpr_free_object(sexpr1);

    /* manually free sexpr2 and sexpr3 */
    free(sexpr2->data.value);
    free(sexpr2);
    free(sexpr3->data.value);
    free(sexpr3);
    
    /* cmocka will validate allocated/released chunks */
}

UnitTest(sexpr_car_should_return_data_pointer_if_type_is_cons)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.data.sexpr = (struct sexpression *)12345;
    
    assert_ptr_equal(sexpr_car(&dummy), (struct sexpression *) 12345);
}

UnitTest(sexpr_car_should_return_null_if_type_is_not_cons)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_STRING;
    dummy.data.sexpr = (struct sexpression *)12345;
    
    assert_null(sexpr_car(&dummy));
}
    
UnitTest(sexpr_cdr_should_return_cdr_pointer_if_type_is_cons)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.cdr.sexpr =  (struct sexpression *) 12345;
    
    assert_ptr_equal(sexpr_cdr(&dummy), (struct sexpression *) 12345);
}
    
UnitTest(sexpr_cdr_should_return_null_if_type_is_not_cons)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_PRIMITIVE;
    dummy.cdr.sexpr = (struct sexpression *) 12345;
    
    assert_null(sexpr_cdr(&dummy));
}


UnitTest(sexpr_value_should_return_value_pointer_if_type_is_value)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_STRING;
    dummy.data.value = (wchar_t *) 12345;
    
    assert_ptr_equal(sexpr_value(&dummy), (wchar_t *) 12345);
}
    
UnitTest(sexpr_value_should_return_null_if_type_is_not_value)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.data.value = L"12345";
    
    assert_null(sexpr_value(&dummy));
}

UnitTest(sexpr_type_should_return_sexpression_type)
{
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_FUNCTION;
    
    assert_int_equal(sexpr_type(&dummy), ST_FUNCTION);
}
    
UnitTest(sexpr_value_should_return_NIL_if_sexpression_is_null)
{
    assert_int_equal(sexpr_type(NULL), ST_NIL);
}

UnitTest(sexpr_is_nil_should_return_true_if_type_is_nil_or_sexpression_is_null)
{
    struct sexpression dummy;
    
    assert_true(sexpr_is_nil(NULL));
    
    dummy.type = ST_NIL;
    assert_true(sexpr_is_nil(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_nil(&dummy));
    
    dummy.type = ST_STRING;
    assert_false(sexpr_is_nil(&dummy));
    
    dummy.type = ST_SYMBOL;
    assert_false(sexpr_is_nil(&dummy));
    
    dummy.type = ST_PRIMITIVE;
    assert_false(sexpr_is_nil(&dummy));
    
    dummy.type = ST_FUNCTION;
    assert_false(sexpr_is_nil(&dummy));
}

UnitTest(sexpr_is_cons_should_return_true_if_type_is_cons)
{
    struct sexpression dummy;
    
    assert_false(sexpr_is_cons(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_cons(&dummy));
    
    dummy.type = ST_CONS;
    assert_true(sexpr_is_cons(&dummy));
    
    dummy.type = ST_SYMBOL;
    assert_false(sexpr_is_cons(&dummy));
    
    dummy.type = ST_PRIMITIVE;
    assert_false(sexpr_is_cons(&dummy));
}

UnitTest(sexpr_is_string_should_return_true_if_type_is_value)
{
    struct sexpression dummy;
    
    assert_false(sexpr_is_string(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_string(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_string(&dummy));
    
    dummy.type = ST_STRING;
    assert_true(sexpr_is_string(&dummy));
    
    dummy.type = ST_SYMBOL;
    assert_false(sexpr_is_string(&dummy));
    
    dummy.type = ST_PRIMITIVE;
    assert_false(sexpr_is_string(&dummy));
}

UnitTest(sexpr_is_symbol_should_return_true_if_type_is_value)
{
    struct sexpression dummy;
    
    assert_false(sexpr_is_symbol(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_symbol(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_symbol(&dummy));
    
    dummy.type = ST_STRING;
    assert_false(sexpr_is_symbol(&dummy));
    
    dummy.type = ST_SYMBOL;
    assert_true(sexpr_is_symbol(&dummy));
    
    dummy.type = ST_PRIMITIVE;
    assert_false(sexpr_is_symbol(&dummy));
}

UnitTest(sexpr_is_primitive_should_return_true_if_type_is_ptr)
{
    struct sexpression dummy;
    
    assert_false(sexpr_is_primitive(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_primitive(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_primitive(&dummy));
    
    dummy.type = ST_STRING;
    assert_false(sexpr_is_primitive(&dummy));
    
    dummy.type = ST_PRIMITIVE;
    assert_true(sexpr_is_primitive(&dummy));
}

/* couple of tests for sexpr_equal */
UnitTest(sexpr_equal_should_return_true_if_both_sexpressions_are_equal) {
    struct sexpression * a;
    struct sexpression * b;
    
    assert_true(sexpr_equal(NULL, NULL));
    
    a = sexpr_create_cstring(L"EQUAL");
    assert_true(sexpr_equal(a, a));
    b = sexpr_create_cstring(L"EQUAL");
    assert_true(sexpr_equal(a, b));
    
    a = sexpr_cons(sexpr_create_cstring(L"VALUE"), sexpr_cons(a, NULL));
    b = sexpr_cons(sexpr_create_cstring(L"VALUE"), sexpr_cons(b, NULL));
    assert_true(sexpr_equal(a, b));
    
    sexpr_free(a);
    sexpr_free(b);
    
}

UnitTest(sexpr_equal_should_return_false_if_both_sexpressions_are_not_equal) {
    struct sexpression * a;
    struct sexpression * b;
    
    a = sexpr_create_cstring(L"EQUAL");
    b = sexpr_create_cstring(L"DIFFERENT");
    assert_false(sexpr_equal(NULL, a));
    assert_false(sexpr_equal(a, NULL));
    
    assert_false(sexpr_equal(a, b));
    assert_false(sexpr_equal(b, a));
    
    a = sexpr_cons(sexpr_create_cstring(L"VALUE"), sexpr_cons(a, NULL));
    b = sexpr_cons(sexpr_create_cstring(L"VALUE"), sexpr_cons(b, NULL));
    assert_false(sexpr_equal(a, b));
    
    sexpr_free(a);
    sexpr_free(b);
    
    a = sexpr_create_cstring(L"VALUE");
    b = sexpr_cons(NULL, sexpr_create_cstring(L"VALUE"));
    assert_false(sexpr_equal(a, b));
    
    sexpr_free(a);
    sexpr_free(b);
    
}


/* few tests for sexpr_reverse */
UnitTest(sexpr_reverse_should_return_same_sexpression_if_param_is_not_cons) {
    struct sexpression * a;
    struct sexpression * b;
    
    assert_null(sexpr_reverse(NULL));
    
    a = sexpr_create_cstring(L"EQUAL");
    b = sexpr_reverse(a);
    assert_ptr_equal(a, b);
    
    sexpr_free(a);
    
}

UnitTest(sexpr_reverse_should_should_swap_car_with_cdr_if_param_is_a_pair) {
    struct sexpression * a;
    struct sexpression * b;
    struct sexpression * c;
    struct sexpression * d;
    
    a = sexpr_create_cstring(L"CAR");
    b = sexpr_create_cstring(L"CDR");
    c = sexpr_cons(a, b);
    
    d = sexpr_reverse(c);
    
    assert_ptr_equal(c, d);
    
    assert_ptr_equal(sexpr_car(d), b);
    assert_ptr_equal(sexpr_cdr(d), a);
    
    sexpr_free(d);
    
}

UnitTest(sexpr_reverse_should_should_return_same_param_if_param_is_len_1) {
    struct sexpression * sexpr;
    struct sexpression * expected;
    struct sexpression * actual;

    expected = sexpr_cons(sexpr_create_cstring(L"CAR"), NULL);
    sexpr = sexpr_cons(sexpr_create_cstring(L"CAR"), NULL);
    
    actual = sexpr_reverse(sexpr);
    
    assert_ptr_equal(actual, sexpr);
    /* also mitigate bug in sexpr_reverse  with size 1 list*/
    assert_sexpr_equal(expected, actual);
    
    sexpr_free(actual);
    sexpr_free(expected);
    
}

UnitTest(sexpr_reverse_should_should_return_a_reversed_list_if_param_is_a_list) {
    struct sexpression * sexpr;
    struct sexpression * expected;
    struct sexpression * actual;
    
    sexpr = sexpr_cons(sexpr_create_csymbol(L"A"), sexpr_cons(sexpr_create_csymbol(L"B"), sexpr_cons(sexpr_create_csymbol(L"C"), sexpr_cons(sexpr_create_csymbol(L"D"), NULL))));
    expected = sexpr_cons(sexpr_create_csymbol(L"D"), sexpr_cons(sexpr_create_csymbol(L"C"), sexpr_cons(sexpr_create_csymbol(L"B"), sexpr_cons(sexpr_create_csymbol(L"A"),NULL))));
    
    actual = sexpr_reverse(sexpr);
    
    assert_sexpr_equal(actual, expected);
    
    sexpr_free(actual);
    sexpr_free(expected);
    
}

UnitTest(create_cons_should_create_list_and_set_correct_length) {
    struct sexpression * sexpr;
    
    sexpr = sexpr_cons(sexpr_create_csymbol(L"hey"), 
                sexpr_cons(sexpr_create_csymbol(L"y-o-u"),
                    sexpr_cons(sexpr_create_cstring(L"check them dubs"),
                        sexpr_cons(sexpr_create_csymbol(L"123"),NULL))));
    
    assert_int_equal(sexpr_length(sexpr), 4);
    
    sexpr_free(sexpr);
}
