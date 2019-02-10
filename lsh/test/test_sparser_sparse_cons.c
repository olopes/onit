/* prod code includes */
#include "sparser.h"
#include "sparser_privates.h"
#include "wcstr_sparser_adapter.h"
#include "assert_sexpr.c"

/* mocks and stubs */
int 
sparse_object(struct sparse_ctx * ctx, struct sexpression ** obj) {
    *obj = mock_ptr_type(struct sexpression *);
    return mock();
}


/*
what  should happen?
These are the requirements:

    () -> nil
    (x) -> cons(symbol(x), nil)
    (x . y) -> cons(symbol(x), symbol(y))
    (x . y . z) -> error
    (x y z) -> cons(symbol(x), cons(symbol(y), cons(symbol(z), nil)))
    create hint with list size
*/

void sparse_cons_should_return_null_when_input_is_empty_paren(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * obj;
    int return_value;
    
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_PAREN);
    
    return_value = sparse_cons(&ctx, &obj);
    
    assert_null(obj);
    assert_int_equal(return_value, SPARSE_OK);
}

void sparse_cons_should_return_simplest_list_when_input_is_single_value(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    struct sexpression * mock_value;
    struct sexpression * mock_list;
    int return_value;
    
    mock_value = sexpr_create_cstr(L"CORRECT");
    mock_list  = sexpr_cons(mock_value, NULL);
    
    will_return(sparse_object, mock_value);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_PAREN);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_sexpr_equal(mock_list, actual_obj);
    assert_int_equal(return_value, SPARSE_OK);
    
    sexpr_free_pair(mock_list);
    sexpr_free(actual_obj);
}

void sparse_cons_should_return_pair_when_input_has_two_values_separated_by_dot(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    struct sexpression * mock_car;
    struct sexpression * mock_cdr;
    struct sexpression * mock_cons;
    int return_value;
    
    mock_car = sexpr_create_cstr(L"CORRECT CAR");
    mock_cdr = sexpr_create_cstr(L"CORRECT CDR");
    mock_cons  = sexpr_cons(mock_car, mock_cdr);
    
    will_return(sparse_object, mock_car);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_DOT_SYM);
    will_return(sparse_object, mock_cdr);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_PAREN);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_sexpr_equal(mock_cons, actual_obj);
    assert_int_equal(return_value, SPARSE_OK);
    
    sexpr_free_pair(mock_cons);
    sexpr_free(actual_obj);
}

void sparse_cons_should_return_error_when_input_has_multiple_dots(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    struct sexpression * mock_obj1;
    struct sexpression * mock_obj2;
    struct sexpression * mock_obj3;
    int return_value;
    
    mock_obj1 = sexpr_create_cstr(L"CORRECT 1");
    mock_obj2 = sexpr_create_cstr(L"CORRECT 2");
    mock_obj3 = sexpr_create_cstr(L"CORRECT 3");
    
    
    will_return(sparse_object, mock_obj1);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_DOT_SYM);
    will_return(sparse_object, mock_obj2);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_DOT_SYM);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_null(actual_obj);
    assert_int_equal(return_value, SPARSE_BAD_SYM);
    
    /* the other objects will be freed by sparse_cons */
    sexpr_free(mock_obj3);
}


void sparse_cons_should_return_error_when_input_dot_does_not_follow_real_symbol(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    struct sexpression * mock_obj;
    int return_value;
    
    mock_obj = sexpr_create_cstr(L"CORRECT 1");
    
    will_return(sparse_object, mock_obj);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_DOT_SYM);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_DOT_SYM);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_null(actual_obj);
    assert_int_equal(return_value, SPARSE_BAD_SYM);
    
}

void sparse_cons_should_return_list_when_input_has_multiple_values(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    struct sexpression * mock_obj1;
    struct sexpression * mock_obj2;
    struct sexpression * mock_obj3;
    struct sexpression * mock_list1;
    struct sexpression * mock_list2;
    struct sexpression * mock_list3;
    int return_value;
    
    mock_obj1 = sexpr_create_cstr(L"CORRECT 1");
    mock_obj2 = sexpr_create_cstr(L"CORRECT 2");
    mock_obj3 = sexpr_create_cstr(L"CORRECT 3");
    
    mock_list3 = sexpr_cons(mock_obj3, NULL);
    mock_list2 = sexpr_cons(mock_obj2, mock_list3);
    mock_list1 = sexpr_cons(mock_obj1, mock_list2);
    
    
    will_return(sparse_object, mock_obj1);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, mock_obj2);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, mock_obj3);
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_PAREN);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_sexpr_equal(mock_list1, actual_obj);
    assert_int_equal(return_value, SPARSE_OK);
    
    /* the other objects will be freed by sparse_cons */
    sexpr_free_pair(mock_list1);
    sexpr_free_pair(mock_list2);
    sexpr_free_pair(mock_list3);
    sexpr_free(actual_obj);
}

void sparse_cons_should_create_size_hints_on_parsed_value(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    
    will_return(sparse_object, sexpr_create_cstr(L"CORRECT 1"));
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, sexpr_create_cstr(L"CORRECT 2"));
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, sexpr_create_cstr(L"CORRECT 3"));
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_PAREN);
    
    sparse_cons(&ctx, &actual_obj);
    
    assert_int_equal(actual_obj->len, 3);
    
    /* the other objects will be freed by sparse_cons */
    sexpr_free(actual_obj);
}

void sparse_cons_should_return_bad_sym_when_input_contains_incomplete_list(void ** param) {
    struct sparse_ctx ctx;
    struct sexpression * actual_obj;
    int return_value;
    
    will_return(sparse_object, sexpr_create_cstr(L"BAD LIST"));
    will_return(sparse_object, SPARSE_OK);
    will_return(sparse_object, NULL);
    will_return(sparse_object, SPARSE_EOF);
    
    return_value = sparse_cons(&ctx, &actual_obj);
    
    assert_null(actual_obj);
    assert_int_equal(return_value, SPARSE_BAD_SYM);
    
}

/* These functions will be used to initialize
   and clean resources up after each test run */
    
int setup (void ** state)
{
    return 0;
}

int teardown (void ** state)
{
    return 0;
}


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test (sparse_cons_should_return_null_when_input_is_empty_paren),
        cmocka_unit_test (sparse_cons_should_return_simplest_list_when_input_is_single_value),
        cmocka_unit_test (sparse_cons_should_return_pair_when_input_has_two_values_separated_by_dot),
        cmocka_unit_test (sparse_cons_should_return_error_when_input_has_multiple_dots),
        cmocka_unit_test (sparse_cons_should_return_error_when_input_dot_does_not_follow_real_symbol),
        cmocka_unit_test (sparse_cons_should_return_list_when_input_has_multiple_values),
        cmocka_unit_test (sparse_cons_should_create_size_hints_on_parsed_value),
        cmocka_unit_test (sparse_cons_should_return_bad_sym_when_input_contains_incomplete_list),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
