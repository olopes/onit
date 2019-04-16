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
    
static void sctx_do_nothing(void ** param)
{
    struct sctx * sctx = create_new_sctx(arguments, environment);
    release_sctx(sctx);
    assert_true(1);
}

static void sctx_register_new_symbol(void ** param)
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
static void sctx_should_call_gc_when_heap_is_full(void ** param)
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


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test (sctx_do_nothing),
        cmocka_unit_test (sctx_register_new_symbol), 
        cmocka_unit_test (sctx_should_call_gc_when_heap_is_full), 
    };

    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests_name (__FILE__, tests, NULL, NULL);

    return count_fail_tests;
}
