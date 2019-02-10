$(shell mkdir -p build/test > /dev/null)
$(shell mkdir -p build/.d/build/test   > /dev/null)

DEPDIR = build/.d
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

CMOCKA_ROOT=../cmocka

CFLAGS=-O2 -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -DWEAK_FOR_UNIT_TEST=\ 
#LDFLAGS=-static 
LDFLAGS =  

TEST_CFLAGS = -g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -Isrc -I$(CMOCKA_ROOT)/include -include test/test_definitions.h
TEST_LDFLAGS = -lcmocka -L$(CMOCKA_ROOT)/lib

TESTS_PROD_SRC = $(wildcard src/*.c)
TESTS_PROD_OBJ = $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC)))
TESTS_EXECS = $(patsubst %.c,build/%.t,$(wildcard test/test*.c))
TESTS = $(patsubst %.t,%.pass,$(TESTS_EXECS))


all: build/ostr build/sexpr tests 

build/main.c:
	echo 'int MAIN(int a, char ** b); int main(int a, char**b) { return MAIN(a,b); }' > build/main.c

build/ostr: build/ostr.o build/ostr_main.o build/main.c
	gcc -DMAIN=ostr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/sexpr: build/sexpr.o build/sexpr_main.o build/sexpr_stack.o build/svisitor.o build/main.c
	gcc -DMAIN=sexpr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/%.o: src/%.c
	gcc -c $(CFLAGS) -o $@ $^
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

clean:
	rm -r -f -- build *.gcda *.gcno

# Unit tests

build/test/%.o: src/%.c
	mkdir -p build/test
	gcc $(TEST_CFLAGS) -c -o $@ $<

build/test/%.t: test/%.c $(TESTS_PROD_OBJ)
	gcc $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^

build/test/test_ostr.t: test/test_ostr.c $(TESTS_PROD_OBJ)
	gcc $(TEST_LDFLAGS) -Wl,--wrap=fputwc $(TEST_CFLAGS) -o $@ $^

build/test/%.pass: build/test/%.t
	# CMOCKA_MESSAGE_OUTPUT=XML CMOCKA_XML_FILE=$(patsubst %.t,%.xml,$<) $<
	LD_LIBRARY_PATH=$(CMOCKA_ROOT)/lib $<
	touch $@

tests: $(TESTS)

coverage: tests

.PHONY: all tests coverage clean

.PRECIOUS: build/test/%.t

