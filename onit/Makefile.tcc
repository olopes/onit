CFLAGS=-g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -DWEAK_FOR_UNIT_TEST=\ 
LDFLAGS=-static 

TEST_CFLAGS=-g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -Isrc -fPIC -include test/test_definitions.h
TEST_LDFLAGS=-lcmocka -Wl,--wrap=fgetwc -Wl,--wrap=getwc -Wl,--wrap=ungetwc -Wl,--wrap=fputwc 

#PROD_CODE_TO_TEST=src/ostr.c src/sexpr.c src/sexpr_stack.c src/sobj.c src/svisitor.c src/sparser.c src/eval.c
TESTS_PROD_SRC=src/ostr.c src/sexpr.c src/sexpr_stack.c src/sobj.c src/svisitor.c src/sparser.c
TESTS_PROD_OBJ := $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC))) build/test/wrap_fn.o
TESTS := $(patsubst %.c,build/%.t,$(wildcard test/test*.c))

all: build build/onit tests 

build:
	mkdir -p build/test

build/onit: src/onit.c
	cc $(LDFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -r -f -- build *.gcda *.gcno

# Unit tests
empty:=
and:=$(empty) && $(empty)
space:=$(empty) $(empty)

tests: $(TESTS)
	(f=0;for t in $^ ; do echo "Running $$t ..." ; $$t ; f=`expr $$f + $$?` ; done; exit $$f)

build/test/wrap_fn.o: test/wraps.c
	mkdir -p build/test
	cc $(TEST_CFLAGS) -c -o $@ $<

build/test/%.o: src/%.c
	mkdir -p build/test
	cc $(TEST_CFLAGS) -c -o $@ $<

build/test/%.t: test/%.c $(TESTS_PROD_OBJ)
	cc $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^

coverage: tests

.PHONY: all tests coverage clean


# dependencies

src/onit.c: src/onit.h

