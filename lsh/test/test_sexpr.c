/* prod code includes */
#include <wctype.h>
#include <wchar.h>
#include <string.h>
#include "sexpr.h"

void sexpr_cons_should_alloc_new_sexpression(void ** param)
{
    (void) param; /* unused */
    struct sexpression * sexpr;
    struct sexpression dummy_car;
    struct sexpression dummy_cdr;
    
    sexpr = sexpr_cons(&dummy_car, &dummy_cdr);
    
    assert_non_null(sexpr);
    assert_ptr_equal(sexpr->data, &dummy_car);
    assert_ptr_equal(sexpr->cdr, &dummy_cdr);
    assert_int_equal(sexpr->type, ST_CONS);
    
    free(sexpr);
}

void sexpr_create_value_should_alloc_new_sexpression_and_alloc_value_string(void ** param)
{
    (void) param; /* unused */
    struct sexpression * sexpr;
    
    sexpr = sexpr_create_value(L"TEST", 4);
    
    assert_non_null(sexpr);
    assert_int_equal(sexpr->type, ST_VALUE);
    assert_int_equal(sexpr->len, 4);
    assert_true(wcsncmp(L"TEST", (wchar_t *)sexpr->data, 4) == 0);
    
    free(sexpr->data);
    free(sexpr);
}

void sexpr_free_value_should_release_sexpression_recursively(void ** param)
{
    (void) param; /* unused */
    struct sexpression * sexpr;
    
    /* alloc list (CAR CDR) */
    sexpr = sexpr_cons(
        sexpr_create_value(L"CAR", 3),
        sexpr_cons(
            sexpr_create_value(L"CDR", 3), NULL
        )
    );
    
    sexpr_free(sexpr);

    /* cmocka will validate allocated/released chunks */
}

void sexpr_free_pair_value_should_release_only_the_first_pair(void ** param)
{
    (void) param; /* unused */
    struct sexpression * sexpr1;
    struct sexpression * sexpr2;
    struct sexpression * sexpr3;
    
    /* alloc list (CAR CDR) */
    sexpr1 = sexpr_cons(
        sexpr2 = sexpr_create_value(L"CAR", 3),
        sexpr3 = sexpr_create_value(L"CDR", 3)
    );
    
    /* only sexpr_cons pair will be freed */
    sexpr_free_pair(sexpr1);

    /* manually free sexpr2 and sexpr3 */
    free(sexpr2->data);
    free(sexpr2);
    free(sexpr3->data);
    free(sexpr3);
    
    /* cmocka will validate allocated/released chunks */
}

void sexpr_free_pair_value_should_not_release_sexpression_if_not_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression * sexpr;
    
    sexpr = sexpr_create_value(L"CAR", 3);
    
    /* only sexpr_cons pair will be freed */
    sexpr_free_pair(sexpr);

    /* manually free sexpr */
    free(sexpr->data);
    free(sexpr);
    
    /* cmocka will validate allocated/released chunks */
}

void sexpr_car_should_return_data_pointer_if_type_is_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.data = (void *)12345;
    
    assert_ptr_equal(sexpr_car(&dummy), 12345);
}
    
void sexpr_car_should_return_null_if_type_is_not_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_VALUE;
    dummy.data = (void *)12345;
    
    assert_null(sexpr_car(&dummy));
}
    
void sexpr_cdr_should_return_cdr_pointer_if_type_is_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.cdr =  (struct sexpression *) 12345;
    
    assert_ptr_equal(sexpr_cdr(&dummy), 12345);
}
    
void sexpr_cdr_should_return_null_if_type_is_not_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_VALUE;
    dummy.cdr = (struct sexpression *) 12345;
    
    assert_null(sexpr_cdr(&dummy));
}


void sexpr_value_should_return_data_pointer_if_type_is_value(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_VALUE;
    dummy.data = (void *)12345;
    
    assert_ptr_equal(sexpr_value(&dummy), 12345);
}
    
void sexpr_value_should_return_null_if_type_is_not_value(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_CONS;
    dummy.data = (void *)12345;
    
    assert_null(sexpr_value(&dummy));
}

void sexpr_type_should_return_sexpression_type(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;

    memset(&dummy, 0, sizeof(dummy));
    dummy.type=ST_VALUE;
    
    assert_int_equal(sexpr_type(&dummy), ST_VALUE);
}
    
void sexpr_value_should_return_NIL_if_sexpression_is_null(void ** param)
{
    (void) param; /* unused */
    
    assert_int_equal(sexpr_type(NULL), ST_NIL);
}

void sexpr_is_nil_should_return_true_if_type_is_nil_or_sexpression_is_null(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;
    
    assert_true(sexpr_is_nil(NULL));
    
    dummy.type = ST_NIL;
    assert_true(sexpr_is_nil(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_nil(&dummy));
    
    dummy.type = ST_VALUE;
    assert_false(sexpr_is_nil(&dummy));
}

void sexpr_is_cons_should_return_true_if_type_is_cons(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;
    
    assert_false(sexpr_is_cons(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_cons(&dummy));
    
    dummy.type = ST_CONS;
    assert_true(sexpr_is_cons(&dummy));
    
    dummy.type = ST_VALUE;
    assert_false(sexpr_is_cons(&dummy));
}

void sexpr_is_value_should_return_true_if_type_is_value(void ** param)
{
    (void) param; /* unused */
    struct sexpression dummy;
    
    assert_false(sexpr_is_value(NULL));
    
    dummy.type = ST_NIL;
    assert_false(sexpr_is_value(&dummy));
    
    dummy.type = ST_CONS;
    assert_false(sexpr_is_value(&dummy));
    
    dummy.type = ST_VALUE;
    assert_true(sexpr_is_value(&dummy));
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
        cmocka_unit_test (sexpr_cons_should_alloc_new_sexpression),
        cmocka_unit_test (sexpr_create_value_should_alloc_new_sexpression_and_alloc_value_string),
        cmocka_unit_test (sexpr_free_value_should_release_sexpression_recursively),
        cmocka_unit_test (sexpr_car_should_return_data_pointer_if_type_is_cons),
        cmocka_unit_test (sexpr_car_should_return_null_if_type_is_not_cons),
        cmocka_unit_test (sexpr_cdr_should_return_cdr_pointer_if_type_is_cons),
        cmocka_unit_test (sexpr_cdr_should_return_null_if_type_is_not_cons),
        cmocka_unit_test (sexpr_value_should_return_data_pointer_if_type_is_value),
        cmocka_unit_test (sexpr_value_should_return_null_if_type_is_not_value),
        cmocka_unit_test (sexpr_type_should_return_sexpression_type),
        cmocka_unit_test (sexpr_value_should_return_NIL_if_sexpression_is_null),
        cmocka_unit_test (sexpr_is_nil_should_return_true_if_type_is_nil_or_sexpression_is_null),
        cmocka_unit_test (sexpr_is_cons_should_return_true_if_type_is_cons),
        cmocka_unit_test (sexpr_is_value_should_return_true_if_type_is_value),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}

