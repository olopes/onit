#include "test_definitions.h"
#include "sexpr_stack.h"

UnitTest(sexpr_stack_should_work_like_a_stack) {
    struct sexpression * stack = NULL;
    void * data1 = "DATA1";
    void * data2 = "DATA2";
    
    assert_false(sexpr_can_pop(&stack));
    
    sexpr_push(&stack, data1);
    sexpr_push(&stack, data2);
    
    assert_true(sexpr_can_pop(&stack));
    
    assert_ptr_equal(sexpr_peek(&stack), data2);
    
    assert_ptr_equal(sexpr_pop(&stack), data2);
    
    assert_true(sexpr_can_pop(&stack));
    
    assert_ptr_equal(sexpr_pop(&stack), data1);
    
    assert_false(sexpr_can_pop(&stack));
    
}

