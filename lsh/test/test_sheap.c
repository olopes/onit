/* prod code includes */
#include "test_definitions.h"
#include "sexpr.h"
#include "sheap.h"

static void test_create_release_sheap(void ** state) {
    struct sheap * heap;
    
    heap = new_sheap();
    
    
    assert_non_null(heap);
    assert_non_null(heap->data);
    assert_int_equal(heap->size, 0);
    
    /* don't test initial capacity because it might change in the future */
    
    free_sheap(heap);
    
}

static void sheap_is_full_should_return_false_if_heap_is_null(void ** state) {

    assert_false(sheap_is_full(NULL));
    
}

static void sheap_is_full_should_return_false_if_size_lt_capacity(void ** state) {
    struct sheap heap;
    
    heap.capacity = 100;
    heap.size = 0;
    assert_false(sheap_is_full(&heap));
    
}

static void sheap_is_full_should_return_true_if_size_ge_capacity(void ** state) {
    struct sheap heap;
    
    heap.capacity = 100;
    
    heap.size = 100;
    assert_true(sheap_is_full(&heap));
    
    heap.size = 101;
    assert_true(sheap_is_full(&heap));
    
}

static void sheap_capacity_should_return_the_heap_capacity(void ** state) {
    struct sheap heap;
    
    heap.capacity = 100;
    assert_int_equal(sheap_capacity(&heap), 100);
    
}

static void sheap_capacity_should_return_0_if_heap_is_null(void ** state) {

    assert_int_equal(sheap_capacity(NULL), 0);
    
}
    
static void sheap_size_should_return_the_heap_size(void ** state) {
    struct sheap heap;
    
    heap.size = 123;
    assert_int_equal(sheap_size(&heap), 123);
    
}

static void sheap_size_should_return_0_if_heap_is_null(void ** state) {

    assert_int_equal(sheap_size(NULL), 0);
    
}

static void sheap_insert_should_return_SHEAP_ERROR_if_heap_is_null(void ** state) {
    struct sexpression sexpr;
    
    assert_int_equal(sheap_insert(NULL, &sexpr), SHEAP_ERROR);

}

static void sheap_insert_should_return_SHEAP_NULL_if_sexpr_is_null(void ** state) {
    struct sheap heap;
    
    assert_int_equal(sheap_insert(&heap, NULL), SHEAP_NULL);
    
}
    
static void sheap_insert_should_return_SHEAP_FULL_if_sexpr_is_full(void ** state) {
    struct sheap heap;
    struct sexpression sexpr;
    
    heap.capacity = heap.size = 100;
    
    assert_int_equal(sheap_insert(&heap, &sexpr), SHEAP_FULL);
    
}

static void sheap_insert_should_put_sexpr_at_first_position_if_heap_is_empty(void ** state) {
    struct sheap heap;
    struct sexpression sexpr;
    struct sexpression * data [100];
    
    heap = (struct sheap){
        .capacity = 100,
        .size = 0,
        .data = data,
    };
    
    memset(data, 0, sizeof(struct sexpression * )*100);
    
    assert_int_equal(sheap_insert(&heap, &sexpr), SHEAP_OK);
    
    assert_ptr_equal(data[0], &sexpr);
}
    
static void sheap_insert_should_preserve_heap_property(void ** state) {
    struct sheap heap;
    struct sexpression * data [100];
    struct sexpression * expected [8];
    
    expected[0] = (struct sexpression *) 20;
    expected[1] = (struct sexpression *) 15;
    expected[2] = (struct sexpression *) 17;
    expected[3] = (struct sexpression *) 6;
    expected[4] = (struct sexpression *) 9;
    expected[5] = (struct sexpression *) 7;
    expected[6] = (struct sexpression *) 10;
    
    data[0] = (struct sexpression *) 17;
    data[1] = (struct sexpression *) 15;
    data[2] = (struct sexpression *) 10;
    data[3] = (struct sexpression *) 6;
    data[4] = (struct sexpression *) 9;
    data[5] = (struct sexpression *) 7;
    
    heap = (struct sheap){
        .capacity = 100,
        .size = 6,
        .data = data,
    };
    
    assert_int_equal(sheap_insert(&heap, (struct sexpression *) 20), SHEAP_OK);
    
    assert_int_equal(memcmp(expected, data, sizeof(struct sexpression*)*7), 0);
}

static void test_callback(void * param, struct sexpression * value);
static void sheap_visit_should_call_the_callback_function_for_all_elements(void ** param) {
    int visited [] = {0,0,0,0};
    struct sheap heap;
    struct sexpression * data [] = {
        (struct sexpression *)0,
        (struct sexpression *)1,
        (struct sexpression *)2,
        (struct sexpression *)3,
        (struct sexpression *)4,
        (struct sexpression *)5,
        NULL
    };
    int i;
    
    heap.data = data;
    heap.size = 4;
    heap.capacity = 7;
    
    sheap_visit(&heap, visited, test_callback);
    
    for(i = 0; i < 4; i++) {
        assert_true(visited[i]);
    }
}

static void test_callback(void * param, struct sexpression * value) {
    int * visited = (int *) param;
    size_t visited_position = (size_t) value;
    
    assert_in_range(visited_position, 0, 3);
    visited[visited_position] = 1;
}

/*
extern int
sheap_visit(struct sheap * heap, void * param, void (*callback)(void * param, struct sexpression * value));
extern int 
sheap_sort(struct sexpression * sexpr, size_t size, int (*comparator)(struct sexpression * a, struct sexpression * b));
*/

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
    const struct CMUnitTest sheap_test [] =
    {
        cmocka_unit_test (test_create_release_sheap),
        cmocka_unit_test (sheap_is_full_should_return_false_if_heap_is_null),
        cmocka_unit_test (sheap_is_full_should_return_false_if_size_lt_capacity),
        cmocka_unit_test (sheap_is_full_should_return_true_if_size_ge_capacity),
        cmocka_unit_test (sheap_capacity_should_return_0_if_heap_is_null),
        cmocka_unit_test (sheap_capacity_should_return_the_heap_capacity),
        cmocka_unit_test (sheap_size_should_return_0_if_heap_is_null),
        cmocka_unit_test (sheap_size_should_return_the_heap_size),
        cmocka_unit_test (sheap_insert_should_return_SHEAP_ERROR_if_heap_is_null),
        cmocka_unit_test (sheap_insert_should_return_SHEAP_NULL_if_sexpr_is_null),
        cmocka_unit_test (sheap_insert_should_return_SHEAP_FULL_if_sexpr_is_full),
        cmocka_unit_test (sheap_insert_should_put_sexpr_at_first_position_if_heap_is_empty),
        cmocka_unit_test (sheap_insert_should_preserve_heap_property),
        cmocka_unit_test (sheap_visit_should_call_the_callback_function_for_all_elements),
        
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests_name (__FILE__, sheap_test, NULL, NULL);

    return count_fail_tests;
}
