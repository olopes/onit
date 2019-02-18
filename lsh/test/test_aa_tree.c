/* prod code includes */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "aa_tree.h"

#define TEST_DATA_SIZE 26

static wchar_t test_strings[TEST_DATA_SIZE*2];
static struct svalue test_keys [TEST_DATA_SIZE];
static struct sexpression test_values [TEST_DATA_SIZE];

static int insertion_order [TEST_DATA_SIZE];
static int deletion_order [TEST_DATA_SIZE];

static int current_level;
static void assert_visit(struct svalue * key, struct sexpression * value) {
    assert_ptr_equal(key, test_keys+current_level);
    assert_ptr_equal(value, test_values+current_level);
    current_level++;
}

static void test_aa_tree_operations(void ** state) {
    struct aa_tree tree;
    int i;
    
    memset(&tree, 0, sizeof(struct aa_tree));
    
    /* insert everything! */
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        assert_false(aa_insert(&tree, test_keys + insertion_order[i], test_values + insertion_order[i]));
    }
    
    current_level = 0;
    aa_visit(&tree, assert_visit);
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        assert_ptr_equal(aa_delete(&tree, test_keys + deletion_order[i]), test_values + deletion_order[i]);
    }
}


/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
static void shuffle(int *array, size_t n)
{
    size_t i;
    size_t j;
    int t;
    
    for (i = 0; i < n - 1; i++) 
    {
        j = i + rand() / (RAND_MAX / (n - i) + 1);
        t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

/* These functions will be used to initialize
   and clean resources up after each test run */
int setup (void ** state)
{
    int i;
    
    /* init data */
    memset(test_strings, 0, sizeof(test_strings));
    memset(test_keys, 0, sizeof(test_keys));
    memset(test_values, 0, sizeof(test_values));
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        test_strings[i*2] = L'A'+i;
        test_keys[i].len = test_values[i].len = 1;
        test_keys[i].data = test_values[i].data = test_strings+i*2;
        insertion_order[i] = i;
        deletion_order[i] = i;
    }
    
    shuffle(insertion_order, TEST_DATA_SIZE);
    shuffle(deletion_order, TEST_DATA_SIZE);
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
        cmocka_unit_test (test_aa_tree_operations),
        cmocka_unit_test (test_aa_tree_operations),
        cmocka_unit_test (test_aa_tree_operations),
    };

    srand(131071);
    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, setup, teardown);

    return count_fail_tests;
}
