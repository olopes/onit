$(shell mkdir build\test > NUL 2>&1 )
$(shell mkdir build\.d\build\test > NUL 2>&1 )

CC=tcc

DEPDIR = build/.d
#DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
DEPFLAGS = -MD -MF $(DEPDIR)/$*.d

CMOCKA_ROOT=../cmocka

CFLAGS=-O2 -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) "-DWEAK_FOR_UNIT_TEST= "
#LDFLAGS=-static 
LDFLAGS =  
PROD_OBJ = $(patsubst %.c,%.o,$(subst src/,build/,$(wildcard src/*.c))) build/main.c

TEST_CFLAGS = -g -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -Isrc -I$(CMOCKA_ROOT)/include -include test/test_definitions.h
TEST_LDFLAGS = $(CMOCKA_ROOT)/lib/cmocka.o

TESTS_PROD_SRC = $(wildcard src/*.c)
TESTS_PROD_OBJ = $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC)))
TESTS_EXECS = $(patsubst %.c,build/%.t,$(wildcard test/test*.c))
TESTS = $(patsubst %.t,%.pass,$(TESTS_EXECS))

all: build/ostr.exe build/sexpr.exe build/sparse.exe build/lsh.exe tests 

build/main.c:
	echo int MAIN(int a, char ** b); int main(int a, char**b) { return MAIN(a,b); } > $@

build/ostr.exe: $(PROD_OBJ)
	$(CC) -DMAIN=ostr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/sexpr.exe: $(PROD_OBJ)
	$(CC) -DMAIN=sexpr_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/sparse.exe: $(PROD_OBJ)
	$(CC) -DMAIN=sparser_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/lsh.exe: $(PROD_OBJ)
	$(CC) -DMAIN=lsh_main $(CFLAGS) $(LDFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rd /S /Q build

# Unit tests

build/test/%.o: src/%.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

build/test/%.t: test/%.c $(TESTS_PROD_OBJ)
	$(CC) $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^

build/test/test_ostr.t: test/test_ostr.c $(TESTS_PROD_OBJ)
	$(CC) $(TEST_LDFLAGS) -Wl,--wrap=fputwc $(TEST_CFLAGS) -o $@ $^

build/test/%.pass: build/test/%.t
	$<
	echo OK > $@

tests: $(TESTS)

coverage: tests

.PHONY: all tests coverage clean

.PRECIOUS: build/test/%.t

