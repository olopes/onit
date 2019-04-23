/* prod code includes */
#include "test_definitions.h"
#include "svisitor.h"

/* mock implementations */
static int CALL_REAL_SVISITOR = 0;

extern void 
__svisitor(struct sexpression * obj, struct scallback * callback);

void svisitor(struct sexpression * sobj, struct scallback * cb) {
    check_expected(sobj);
    check_expected(cb->context);
    
    if(CALL_REAL_SVISITOR) {
        __svisitor(sobj, cb);
    }
}

/* test case */
UnitTest(dump_sobj_should_call_svisitor) {
    struct sexpression dummy_obj;
    FILE dummy_file;
    
    expect_value(svisitor, sobj, &dummy_obj);
    expect_value(svisitor, cb->context, &dummy_file);
    
    
    dump_sexpr(&dummy_obj, &dummy_file);
    
}
