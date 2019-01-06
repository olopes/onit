
CFLAGS=-g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -DWEAK_FOR_UNIT_TEST
LDFLAGS=-static 

TEST_CFLAGS=-g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -Isrc -DUNIT_TEST "-DWEAK_FOR_UNIT_TEST=__attribute__((weak))" -fprofile-arcs -ftest-coverage
TEST_LDFLAGS=-lcmocka 
#PROD_CODE_TO_TEST=src/ostr.c src/sexpr.c src/sexpr_stack.c src/sobj.c src/svisitor.c src/sparser.c src/eval.c
TESTS_PROD_SRC=src/ostr.c src/sexpr.c src/sexpr_stack.c src/sobj.c src/svisitor.c
TESTS_PROD_OBJ := $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC)))
TESTS := $(patsubst %.c,build/%.t,$(wildcard test/*.c))

all: build/onit build/ostr build/sexpr run-tests 

build/onit: src/onit.c
	cc $(LDFLAGS) $(CFLAGS) -o $@ $^

build/ostr: src/ostr.c src/ostr_main.c
	cc $(LDFLAGS) $(CFLAGS) -o $@ $^

build/sexpr: src/sexpr.c src/sexpr_main.c src/sexpr_stack.c src/sobj.c src/svisitor.c
	cc $(LDFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -r -f -- build/sexpr build/onit build/ostr build/*.o build/test

# Unit tests
run-tests: $(TESTS)

build/test/%.o: src/%.c
	cc $(TEST_CFLAGS) -fPIC -c -o $@ $<

build/test/%.t: test/%.c $(TESTS_PROD_OBJ)
	cc $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^
	$@

.PHONY: all run-tests clean


# dependencies

src/onit.c: src/onit.h

src/ostr.c: src/ostr.h

src/ostr_main.c: src/ostr.h

src/sexpr_main.c: src/stype.h src/sobj.h src/sexpr.h

src/sexpr_stack.c: src/stype.h src/sobj.h src/sexpr.h src/sexpr_stack.h

src/sexpr.c: src/stype.h src/sobj.h src/sexpr.h

src/sobj.c: src/stype.h src/sobj.h

src/sobj.c: src/stype.h src/svisitor.h src/sobj.h src/sexpr.h src/sexpr_stack.h
