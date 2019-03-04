$(shell mkdir -p build/test > /dev/null)
$(shell mkdir -p build/.d/build/test   > /dev/null)

DEPDIR = build/.d
# DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
DEPFLAGS = -MD -MF $(DEPDIR)/$*.Td

CMOCKA_ROOT=../cmocka

CFLAGS=-O2 -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -DWEAK_FOR_UNIT_TEST=\ 
#LDFLAGS=-static 
LDFLAGS =  
PROD_OBJ = $(patsubst %.c,%.o,$(subst src/,build/,$(wildcard src/*.c))) build/main.c

TEST_CFLAGS = -g -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -Isrc -I$(CMOCKA_ROOT)/include -include test/test_definitions.h
# TEST_LDFLAGS = -lcmocka -L$(CMOCKA_ROOT)/lib
TEST_LDFLAGS = ../cmocka/lib/cmocka-linux-gcc.o

TESTS_PROD_SRC = $(wildcard src/*.c)
TESTS_PROD_OBJ = $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC)))
TESTS_EXECS = $(patsubst %.c,build/%.t,$(wildcard test/test*.c))
TESTS = $(patsubst %.t,%.pass,$(TESTS_EXECS))
#CC=../tcc/bin/tcc
CC=gcc

all: build/ostr build/sexpr build/sparse build/lsh tests 

build/main.c:
	echo 'int MAIN(int a, char ** b); int main(int a, char**b) { return MAIN(a,b); }' > build/main.c

build/ostr: $(PROD_OBJ)
	$(CC) -DMAIN=ostr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/sexpr: $(PROD_OBJ)
	$(CC) -DMAIN=sexpr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/sparse: $(PROD_OBJ)
	$(CC) -DMAIN=sparser_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/lsh: $(PROD_OBJ)
	$(CC) -DMAIN=lsh_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) -c $(CFLAGS) -o $@ $^
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

clean:
	rm -r -f -- build *.gcda *.gcno

# Unit tests

build/test/%.o: src/%.c
	mkdir -p build/test
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

build/test/%.t: test/%.c $(TESTS_PROD_OBJ)
	$(CC) $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^

build/test/%.pass: build/test/%.t
	$<
	touch $@

tests: $(TESTS)

coverage: tests

.PHONY: all tests coverage clean

.PRECIOUS: build/test/%.t

