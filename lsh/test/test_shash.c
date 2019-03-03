/* prod code includes */
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sexpr.h"
#include "shash.h"

#define TEST_DATA_SIZE 26

static wchar_t test_strings[TEST_DATA_SIZE*2];
static struct svalue test_keys [TEST_DATA_SIZE];
static struct sexpression test_values [TEST_DATA_SIZE];

static int insertion_order [TEST_DATA_SIZE];
static int deletion_order [TEST_DATA_SIZE];
static int key_visited [TEST_DATA_SIZE];

static int dummy_thing;
static void * test_ctx = &dummy_thing;

static void assert_visit(void * param, struct svalue * key, void * value) {
    size_t index;
    
    /* cool! the compiler handles the alignment stuff */
    index = (size_t) (key-test_keys);
    
    key_visited[index] = 1;
    
    assert_ptr_equal(key, test_keys+index);
    assert_ptr_equal(value, test_values+index);
    assert_ptr_equal(param, test_ctx);

}

static void test_shash_operations(void ** state) {
    struct shash_table hashtable;
    int i;
    int position;
    
    memset(&hashtable, 0, sizeof(struct shash_table));
    
    /* insert everything! */
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        position = insertion_order[i];
        assert_false(shash_insert(&hashtable, &test_keys[position], &test_values[position]));
    }
    
    shash_visit(&hashtable, test_ctx, assert_visit);
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        assert_true(key_visited[i]);
    }
    
    
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        position = deletion_order[i];
        assert_ptr_equal(shash_delete(&hashtable, &test_keys[position]), &test_values[position]);
    }
    
    /* mandatort cleanup */
    shash_free(&hashtable);

}

static void test_shash_internals(void ** state) {
    struct shash_table hashtable;
    struct shash_entry * table;
    int i;
    
    memset(&hashtable, 0, sizeof(struct shash_table));
    
    /* insert everything! */
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        shash_insert(&hashtable, &test_keys[i], &test_values[i]);
    }
    
    /* the hash table will grow twice:
     * - first time after 12 insertions (75% of 16)
     * - second time after 24 insertions (75% of 32)
     * The final size will be 64 */
    assert_int_equal(hashtable.size, 64);
    assert_int_equal(hashtable.load, 26);
    
    
    
    for(i = 0; i < TEST_DATA_SIZE; i++) {
        shash_delete(&hashtable, &test_keys[i]);
    }
    
    assert_int_equal(hashtable.size, 64);
    assert_int_equal(hashtable.load, 0);
   
    /* all keys and values must have been reset to NULL */
    for(i = 0, table = hashtable.table; i < 64; i++, table++) {
        assert_null(table->key);
        assert_null(table->value);
    }
    
    
    /* mandatort cleanup */
    shash_free(&hashtable);

}


static void test_shash_has_key(void ** state) {
    struct shash_table hashtable;
    struct sexpression * found;
    
    memset(&hashtable, 0, sizeof(struct shash_table));
    
    shash_insert(&hashtable, &test_keys[0], &test_values[0]);
    shash_insert(&hashtable, &test_keys[1], &test_values[1]);
    shash_insert(&hashtable, &test_keys[2], &test_values[2]);
    shash_insert(&hashtable, &test_keys[3], &test_values[3]);
    shash_insert(&hashtable, &test_keys[4], &test_values[4]);

    assert_true(shash_has_key(&hashtable, &test_keys[2]));
    
    assert_false(shash_has_key(&hashtable, &test_keys[10]));
    
    found = shash_search(&hashtable, &test_keys[0]);
    
    assert_ptr_equal(found, &test_values[0]);
    
    shash_delete(&hashtable, &test_keys[0]);
    shash_delete(&hashtable, &test_keys[1]);
    shash_delete(&hashtable, &test_keys[2]);
    shash_delete(&hashtable, &test_keys[3]);
    shash_delete(&hashtable, &test_keys[4]);

    /* mandatort cleanup */
    shash_free(&hashtable);
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
        key_visited[i] = 0;
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
        cmocka_unit_test_setup (test_shash_operations, setup),
        cmocka_unit_test_setup (test_shash_operations, setup),
        cmocka_unit_test_setup (test_shash_operations, setup),
        cmocka_unit_test_setup (test_shash_has_key, setup),
        cmocka_unit_test_setup (test_shash_internals, setup),
    };

    srand(131071);
    
    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests =
        cmocka_run_group_tests (tests, NULL, NULL);

    return count_fail_tests;
}