/* prod code includes */
#include "test_definitions.h"
#include "sexpr.h"
#include "aa_tree.h"

#define TEST_DATA_SIZE 26

static wchar_t test_strings[TEST_DATA_SIZE*(TEST_DATA_SIZE+1)];
static struct sexpression * test_keys [TEST_DATA_SIZE];
static struct sexpression * test_values [TEST_DATA_SIZE];

static int insertion_order [TEST_DATA_SIZE];
static int deletion_order [TEST_DATA_SIZE];

static int current_level;
static void * test_ctx = &current_level;

static void assert_visit(void * test_ctx, struct sexpression * key, void * value) {
    assert_ptr_equal(key, test_keys+current_level);
    assert_ptr_equal(value, test_values+current_level);
    current_level++;
}

static void test_aa_tree_operations(void ** state) {
    struct aa_tree tree;
    int i;
    int position;
    
    memset(&tree, 0, sizeof(struct aa_tree));
    
    /* insert everything! */
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        position = insertion_order[i];
        assert_false(aa_insert(&tree, test_keys[position], test_values[position]));
    }
    
    current_level = 0;
    aa_visit(test_ctx, &tree, assert_visit);
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        position = deletion_order[i];
        assert_ptr_equal(aa_delete(&tree, test_keys[position]), test_values[position]);
    }
    
}


static void test_aa_tree_find_with_single_node(void ** state) {
    struct aa_tree tree;
    
    memset(&tree, 0, sizeof(struct aa_tree));
    
    aa_insert(&tree, test_keys[0], test_values[0]);

    assert_ptr_equal(aa_search(&tree, test_keys[0]), test_values[0]);
    
    assert_null(aa_search(&tree, test_keys[10]));
    
    aa_delete(&tree, test_keys[0]);

}

static void test_aa_tree_has_key(void ** state) {
    struct aa_tree tree;
    struct sexpression * found;
    
    memset(&tree, 0, sizeof(struct aa_tree));
    
    aa_insert(&tree, test_keys[0], test_values[0]);
    aa_insert(&tree, test_keys[1], test_values[1]);
    aa_insert(&tree, test_keys[2], test_values[2]);
    aa_insert(&tree, test_keys[3], test_values[3]);
    aa_insert(&tree, test_keys[4], test_values[4]);

    assert_true(aa_has_key(&tree, test_keys[2]));
    
    assert_false(aa_has_key(&tree, test_keys[10]));
    
    found = aa_search(&tree, test_keys[0]);
    
    assert_ptr_equal(found, test_values[0]);
    
    aa_delete(&tree, test_keys[0]);
    aa_delete(&tree, test_keys[1]);
    aa_delete(&tree, test_keys[2]);
    aa_delete(&tree, test_keys[3]);
    aa_delete(&tree, test_keys[4]);

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
    int j;
    wchar_t * str_ptr;
    wchar_t * ptest_strings = test_strings;
    
    /* init data */
    memset(test_strings, 0, sizeof(test_strings));
    memset(test_keys, 0, sizeof(test_keys));
    memset(test_values, 0, sizeof(test_values));
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        str_ptr = ptest_strings;
        for(j = 0; j < i+1; j++, ptest_strings++)
            *ptest_strings = (rand()%2 ? L'A' : L'a')+j;
        *ptest_strings = L'\0';
        ptest_strings++;
        
        test_keys[i] = sexpr_create_value(str_ptr, i+1);
        test_values[i] = sexpr_create_value(str_ptr, i+1);
        insertion_order[i] = i;
        deletion_order[i] = i;
    }
    
    shuffle(insertion_order, TEST_DATA_SIZE);
    shuffle(deletion_order, TEST_DATA_SIZE);
    return 0;
}

int teardown (void ** state)
{
    int i;
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        sexpr_free(test_keys[i]);
        sexpr_free(test_values[i]);
    }
    
    return 0;
}


int main (void)
{
    const struct CMUnitTest tests [] =
    {
        cmocka_unit_test_setup (test_aa_tree_operations, setup),
        cmocka_unit_test_setup (test_aa_tree_operations, setup),
        cmocka_unit_test_setup (test_aa_tree_operations, setup),
        cmocka_unit_test (test_aa_tree_find_with_single_node),
        cmocka_unit_test (test_aa_tree_has_key),
    };

    srand(131071);
    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, NULL, NULL);

    return count_fail_tests;
}
