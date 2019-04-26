/* prod code includes */
#include "test_definitions.h"
#include "sctx.h"

static char * arguments[2] = {
    "arg1",
    NULL
};
static char * environment[4] = {
    "VAR1=VAL1",
    "PATH=/bin:/usr/bin",
    "HOME=/home/user",
    NULL
};

UnitTest(sctx_do_nothing)
{
    struct sctx * sctx = create_new_sctx(arguments, environment);
    release_sctx(sctx);
    assert_true(1);
}

UnitTest(sctx_register_new_symbol)
{
    struct sexpression * key;
    struct sexpression * value;
    struct mem_reference reference;
    struct sctx * sctx = create_new_sctx(arguments, environment);
    
    /* add the new symbol */
    create_stack_reference(sctx, L"VAR_NAME", 8, &reference);
    *reference.value = value = alloc_new_value(sctx, L"THE VALUE", 9);

    
    /* fetch the value using a different name */
    key = alloc_new_value(sctx, L"VAR_NAME", 8);
    
    assert_ptr_equal(value, lookup_name(sctx, key ));
    
    release_sctx(sctx);
}

/** 
 * The default heap size is 32. By definition, the test it will alloc 
 * all test arguments and environment, including the primitives names.
 * This test will force the heap to grow and garbage collector to run 
 */
UnitTest(sctx_should_call_gc_when_heap_is_full)
{
    struct sexpression * unreferenced;
    struct sexpression * referenced;
    struct sexpression * last_one;
    struct mem_reference reference;
    int test_symbol_found;
    int last_symbol_found;
    struct sctx * sctx = create_new_sctx(arguments, environment);
    wchar_t value_text[32];
    size_t i;
    
    
    /* create a reference for the S-Expression ("THE VALUE" . ())  */
    referenced = alloc_new_value(sctx, L"THE VALUE", 9);
    create_stack_reference(sctx, L"VAR_NAME", 8, &reference);
    *reference.value = alloc_new_pair(sctx, referenced, NULL);

    /* alloc random objects that will be garbage collected */
    memset(value_text, 0, sizeof(value_text));
    for(i = sctx->heap.load; i < sctx->heap.size; i++) {
        swprintf(value_text, 32, L"VALUE %u", (unsigned) i);
        unreferenced = alloc_new_value(sctx, value_text, wcslen(value_text));
    }
    
    
    /* alloc a new object that will trigger GC */
    last_one = alloc_new_value(sctx, L"LAST ONE", 8);
    
    
    /* check the heap for preserved and GC references */
    for (i = 0, last_symbol_found = 0, test_symbol_found = 0; i < sctx->heap.size; i++) {
        if (sctx->heap.data[i] == unreferenced) {
            fail_msg("Unexpected reference found in the heap: %p should have been garbage collected", 
                     (void *) unreferenced);
        }
        else if (sctx->heap.data[i] == referenced) {
            /* this one has a reference, it must be preserved. */
            test_symbol_found = 1;
        }
        else if (sctx->heap.data[i] == last_one) {
            /* this one was inserted after the GC run, so it must be there */
            last_symbol_found = 1;
        }
    }
    
    assert_true(test_symbol_found);
    assert_true(last_symbol_found);

    release_sctx(sctx);
}

UnitTest(move_to_heap_should_put_the_give_sexpression_into_the_heap_avoiding_the_garbage_collector) {
    struct sexpression * sexpr1;
    struct sexpression * sexpr2;
    struct sexpression * sexpr3;
    int sexpr1_found;
    int sexpr2_found;
    int sexpr3_found;
    struct sctx * sctx = create_new_sctx(arguments, environment);
    wchar_t value_text[32];
    size_t i;
    
    sexpr3 = sexpr_create_cstr(L"SECOND VALUE");
    sexpr2 = sexpr_cons(sexpr3, NULL);
    sexpr1 = sexpr_cons(sexpr_create_cstr(L"FIRST_VALUE"), sexpr2);
    
    /* alloc random objects until the heap is almost full */
    memset(value_text, 0, sizeof(value_text));
    for(i = sctx->heap.load; i < sctx->heap.size-1; i++) {
        swprintf(value_text, 32, L"VALUE %u", (unsigned) i);
        alloc_new_value(sctx, value_text, wcslen(value_text));
    }
    
    move_to_heap(sctx, sexpr1);
    
    /* check the heap for preserved and GC references */
    sexpr1_found = sexpr2_found = sexpr3_found = 0;
    for (i = 0; i < sctx->heap.load; i++) {
        if (sctx->heap.data[i] == sexpr1) {
            sexpr1_found = 1;
        }
        if (sctx->heap.data[i] == sexpr2) {
            sexpr2_found = 1;
        }
        if (sctx->heap.data[i] == sexpr3) {
            sexpr3_found = 1;
        }
    }
    
    assert_true(sexpr1_found);
    assert_true(sexpr2_found);
    assert_true(sexpr3_found);

    /* the expressions allocated manually will be released by release_sctx() */
    release_sctx(sctx);
}

UnitTest(lookup_name_should_search_primitives_and_all_namespaces) {
    struct mem_reference ref1;
    struct mem_reference ref2;
    struct mem_reference ref3;
    
    struct sctx * sctx = create_new_sctx(arguments, environment);
    enter_namespace(sctx);

    assert_int_equal(create_primitive_reference(sctx, L"pr1", 3, &ref1), SCTX_OK);
    assert_int_equal(create_global_reference(sctx, L"gr2", 3, &ref2), SCTX_OK);
    assert_int_equal(create_stack_reference(sctx, L"sr3", 3, &ref3), SCTX_OK);
    
    enter_namespace(sctx);
    
    *ref1.value = alloc_new_value(sctx, L"gv1", 3);
    *ref2.value = alloc_new_value(sctx, L"gv2", 3);
    *ref3.value = alloc_new_value(sctx, L"sv3", 3);
    
    /* query name spaces */
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"pr1", 3)), *ref1.value);
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"gr2", 3)), *ref2.value);
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"sr3", 3)), *ref3.value);
    
    leave_namespace(sctx);
    leave_namespace(sctx);
    
    release_sctx(sctx);
}

UnitTest(lookup_name_should_return_null_when_reference_out_of_scope) {
    struct mem_reference ref1;
    struct mem_reference ref2;
    struct mem_reference ref3;
    
    struct sctx * sctx = create_new_sctx(arguments, environment);
    enter_namespace(sctx);

    assert_int_equal(create_primitive_reference(sctx, L"pr1", 3, &ref1), SCTX_OK);
    assert_int_equal(create_global_reference(sctx, L"gr2", 3, &ref2), SCTX_OK);
    assert_int_equal(create_stack_reference(sctx, L"sr3", 3, &ref3), SCTX_OK);
    
    enter_namespace(sctx);
    
    *ref1.value = alloc_new_value(sctx, L"gv1", 3);
    *ref2.value = alloc_new_value(sctx, L"gv2", 3);
    *ref3.value = alloc_new_value(sctx, L"sv3", 3);
    
    leave_namespace(sctx);
    leave_namespace(sctx);
    
    /* query name spaces */
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"pr1", 3)), *ref1.value);
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"gr2", 3)), *ref2.value);
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"sr3", 3)), NULL);
    
    release_sctx(sctx);
}

UnitTest(lookup_name_should_search_primitives_first) {
    struct mem_reference ref1;
    struct mem_reference ref2;
    struct mem_reference ref3;
    
    struct sctx * sctx = create_new_sctx(arguments, environment);
    enter_namespace(sctx);

    assert_int_equal(create_primitive_reference(sctx, L"pr1", 3, &ref1), SCTX_OK);
    assert_int_equal(create_global_reference(sctx, L"pr1", 3, &ref2), SCTX_OK);
    assert_int_equal(create_stack_reference(sctx, L"pr1", 3, &ref3), SCTX_OK);
    
    *ref1.value = alloc_new_value(sctx, L"gv1", 3);
    *ref2.value = alloc_new_value(sctx, L"gv2", 3);
    *ref3.value = alloc_new_value(sctx, L"sv3", 3);
    
    /* query name spaces */
    assert_ptr_equal(lookup_name(sctx, alloc_new_value(sctx, L"pr1", 3)), *ref1.value);
    
    leave_namespace(sctx);
    
    release_sctx(sctx);
}

