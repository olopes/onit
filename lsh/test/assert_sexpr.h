#ifndef ASSERT_SEXPR
#define ASSERT_SEXPR

#define assert_sexpr_equal(a, b) \
  { \
    if(!sexpr_equal(a, b)) { \
        fail_msg("%s", "S-Expressions are different"); \
    } \
  }

#endif 
