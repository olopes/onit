/* prod code includes */
#include "test_definitions.h"
#include "sexpr.h"
#include "sheap.h"

#define assert_arrays_equal(a, b, nmemb) _assert_arrays_equal(a, b, nmemb, __FILE__, __LINE__)
#define assert_array_index_equal(a, b, i) \
    _assert_int_equal(cast_ptr_to_largest_integral_type(a[i]), \
                      cast_ptr_to_largest_integral_type(b[i]), \
                      file, line)

static void _assert_arrays_equal(
    struct sexpression ** a, struct sexpression ** b, size_t nmemb,
    const char * const file, const int line) {
    size_t i;
    
    for(i = 0; i < nmemb; i++) {
        assert_array_index_equal(a, b, i);
    }
}

#undef assert_array_index_equal

_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wpointer-to-int-cast\"")
static inline int cast_to_int(struct sexpression * ptr) {
    return (int)ptr;
}
_Pragma("GCC diagnostic pop")

_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"")
static inline struct sexpression * cast_to_sexpr(int value) {
    return (struct sexpression *) value;

}
_Pragma("GCC diagnostic pop")

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
    
    expected[0] = cast_to_sexpr(20);
    expected[1] = cast_to_sexpr(15);
    expected[2] = cast_to_sexpr(17);
    expected[3] = cast_to_sexpr(6);
    expected[4] = cast_to_sexpr(9);
    expected[5] = cast_to_sexpr(7);
    expected[6] = cast_to_sexpr(10);
    
    data[0] = cast_to_sexpr(17);
    data[1] = cast_to_sexpr(15);
    data[2] = cast_to_sexpr(10);
    data[3] = cast_to_sexpr(6);
    data[4] = cast_to_sexpr(9);
    data[5] = cast_to_sexpr(7);
    
    heap = (struct sheap){
        .capacity = 100,
        .size = 6,
        .data = data,
    };
    
    assert_int_equal(sheap_insert(&heap, cast_to_sexpr(20)), SHEAP_OK);
    
    assert_int_equal(memcmp(expected, data, sizeof(struct sexpression*)*7), 0);
}

static void test_callback(void * param, struct sexpression * value);
static void sheap_visit_should_call_the_callback_function_for_all_elements(void ** param) {
    int visited [] = {0,0,0,0};
    struct sheap heap;
    struct sexpression * data [] = {
        cast_to_sexpr(0),
        cast_to_sexpr(1),
        cast_to_sexpr(2),
        cast_to_sexpr(3),
        cast_to_sexpr(4),
        cast_to_sexpr(5),
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


static int 
test_comparator(struct sexpression * a, struct sexpression * b);

static void
sheap_sort_should_return_ERROR_when_array_is_null(void ** param) {
    
    assert_int_equal(sheap_sort(NULL, 8, test_comparator), SHEAP_ERROR);
    
}

static void
sheap_sort_should_return_ERROR_when_comparator_is_null(void ** param) {
    struct sexpression * data_to_sort;
    
    assert_int_equal(sheap_sort(&data_to_sort, 8, NULL), SHEAP_ERROR);
    
}

static void
sheap_sort_should_sort_the_given_array(void ** param) {
    struct sexpression * data_to_sort[] = {
        cast_to_sexpr(7),
        cast_to_sexpr(-4),
        cast_to_sexpr(65),
        cast_to_sexpr(17),
        cast_to_sexpr(87),
        cast_to_sexpr(10),
        cast_to_sexpr(-100),
        cast_to_sexpr(38),
        cast_to_sexpr(7),
    };
    struct sexpression * expected_data[] = {
        cast_to_sexpr(87),
        cast_to_sexpr(65),
        cast_to_sexpr(38),
        cast_to_sexpr(17),
        cast_to_sexpr(10),
        cast_to_sexpr(7),
        cast_to_sexpr(-4),
        cast_to_sexpr(-100),
        cast_to_sexpr(7),
    };
    
    assert_int_equal(sheap_sort(data_to_sort, 8, test_comparator), SHEAP_OK);
    
    assert_arrays_equal(expected_data, data_to_sort, 9);
}

static void
sheap_sort_should_sort_the_given_array0(void ** param) {
    struct sexpression * data_to_sort[] = {
        cast_to_sexpr(7),
        cast_to_sexpr(-4),
        cast_to_sexpr(65),
        cast_to_sexpr(17),
        cast_to_sexpr(87),
        cast_to_sexpr(10),
        cast_to_sexpr(-100),
        cast_to_sexpr(38),
        cast_to_sexpr(7),
    };
    struct sexpression * expected_data[] = {
        cast_to_sexpr(87),
        cast_to_sexpr(65),
        cast_to_sexpr(38),
        cast_to_sexpr(17),
        cast_to_sexpr(10),
        cast_to_sexpr(7),
        cast_to_sexpr(7),
        cast_to_sexpr(-4),
        cast_to_sexpr(-100),
    };
    
    assert_int_equal(sheap_sort(data_to_sort, 9, test_comparator), SHEAP_OK);
    
    assert_arrays_equal(expected_data, data_to_sort, 9);
}

static void
sheap_sort_should_sort_the_given_array1(void ** param) {
    struct sexpression * data_to_sort[] = {
        cast_to_sexpr(87),
    };
    struct sexpression * expected_data[] = {
        cast_to_sexpr(87),
    };
    
    assert_int_equal(sheap_sort(data_to_sort, 1, test_comparator), SHEAP_OK);
    
    assert_arrays_equal(expected_data, data_to_sort, 1);
}

static void
sheap_sort_should_sort_the_given_array2(void ** param) {
    struct sexpression * data_to_sort[] = {
        cast_to_sexpr(87),
        cast_to_sexpr(-10),
    };
    struct sexpression * expected_data[] = {
        cast_to_sexpr(87),
        cast_to_sexpr(-10),
    };
    
    assert_int_equal(sheap_sort(data_to_sort, 2, test_comparator), SHEAP_OK);
    
    assert_arrays_equal(expected_data, data_to_sort, 2);
}

static void
sheap_sort_should_sort_the_given_array3(void ** param) {
    struct sexpression * data_to_sort[] = {
        cast_to_sexpr(-10),
        cast_to_sexpr(87),
    };
    struct sexpression * expected_data[] = {
        cast_to_sexpr(87),
        cast_to_sexpr(-10),
    };
    
    assert_int_equal(sheap_sort(data_to_sort, 2, test_comparator), SHEAP_OK);
    
    assert_arrays_equal(expected_data, data_to_sort, 2);
}



#define TEST_ARRAY_SIZE 100000
static void
sheap_sort_should_sort_the_given_random_array(void ** param) {
    struct sexpression * data_to_sort[TEST_ARRAY_SIZE+2];
    size_t i;
    
    /*
    Prepare an array like this:
    [-1, rand(), rand(), rand(), ... , rand(), -1]
    first and last -1 are the guard values.
    if they are modified, the test must fail
    */
    
    for(i = 0; i < TEST_ARRAY_SIZE; i++) {
        data_to_sort[i+1] = cast_to_sexpr(rand());
    }
    /* set the guards */
    data_to_sort[0] = data_to_sort[TEST_ARRAY_SIZE+1] = cast_to_sexpr(-1);
    
    
    sheap_sort(data_to_sort+1, TEST_ARRAY_SIZE, test_comparator);
    
    for(i = 1; i < TEST_ARRAY_SIZE; i++) {
        assert_true(cast_to_int(data_to_sort[i]) >= cast_to_int(data_to_sort[i+1]));
    }
    
    /* check the guards */
    assert_int_equal(cast_to_int(data_to_sort[0]), -1);
    assert_int_equal(cast_to_int(data_to_sort[TEST_ARRAY_SIZE+1]), -1);

}

static int 
test_comparator(struct sexpression * a, struct sexpression * b) {
    return cast_to_int(b) - cast_to_int(a);
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
        cmocka_unit_test (sheap_sort_should_return_ERROR_when_array_is_null),
        cmocka_unit_test (sheap_sort_should_return_ERROR_when_comparator_is_null),
        cmocka_unit_test (sheap_sort_should_sort_the_given_array),
        cmocka_unit_test (sheap_sort_should_sort_the_given_array0),
        cmocka_unit_test (sheap_sort_should_sort_the_given_array1),
        cmocka_unit_test (sheap_sort_should_sort_the_given_array2),
        cmocka_unit_test (sheap_sort_should_sort_the_given_array3),
        cmocka_unit_test (sheap_sort_should_sort_the_given_random_array),
        cmocka_unit_test (sheap_sort_should_sort_the_given_random_array),
        cmocka_unit_test (sheap_sort_should_sort_the_given_random_array),
        cmocka_unit_test (sheap_sort_should_sort_the_given_random_array),
        cmocka_unit_test (sheap_sort_should_sort_the_given_random_array),
    };

    srand(131071);
    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests_name (__FILE__, sheap_test, NULL, NULL);

    return count_fail_tests;
}
