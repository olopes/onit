$(shell mkdir -p build/test > /dev/null)
$(shell mkdir -p build/.d/build/test > /dev/null)
$(shell mkdir -p build/coverage > /dev/null)
$(shell mkdir -p build/report > /dev/null)
SYSINFO = $(shell uname -mors)

DEPDIR = build/.d
# DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
DEPFLAGS = -MD -MF $(DEPDIR)/$*.Td

CMOCKA_ROOT=../cmocka

CFLAGS=-O2 -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -DWEAK_FOR_UNIT_TEST=\ 
#LDFLAGS=-static 
LDFLAGS =  
PROD_OBJ = $(patsubst %.c,%.o,$(subst src/,build/,$(wildcard src/*.c))) build/main.c

TEST_CFLAGS =-O0 -g -Wall -pedantic -finput-charset=UTF-8 -fPIC $(DEPFLAGS) -Isrc -I$(CMOCKA_ROOT)/include -include test/test_definitions.h --coverage 
# TEST_LDFLAGS = -lcmocka -L$(CMOCKA_ROOT)/lib
TEST_LDFLAGS = ../cmocka/lib/cmocka.o --coverage

TESTS_PROD_SRC = $(wildcard src/*.c)
TESTS_PROD_OBJ = $(patsubst %.c,build/%.o,$(subst src/,test/,$(TESTS_PROD_SRC)))
TEST_MAIN = test/test_main.c
TESTS = $(patsubst %.c,build/%.t,$(filter-out $(TEST_MAIN),$(wildcard test/test*.c)))
TESTS_PASS = $(patsubst %.t,%.gcda,$(TESTS))
TESTS_COV = $(patsubst %.t,%.info,$(TESTS))
#CC=../tcc/bin/tcc
CC=gcc

all: build/ostr build/sexpr build/sparse build/lsh check

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
	rm -r -f -- build

# Unit tests

build/test/%.o: src/%.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

build/test/unit_%.o: test/%.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

build/test/%.t: build/test/unit_%.o $(TEST_MAIN) $(TESTS_PROD_OBJ)
	$(CC) $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $^

build/test/%.gcda: build/test/%.t
	@CMOCKA_MESSAGE_OUTPUT=stdout $<

tests: $(TESTS)

check: tests $(TESTS_PASS)

# Coverage

build/test/%.info: build/test/%.t
	-@lcov -q -z -d build/test 
	-@rm -f build/report/$(basename $(notdir $<)).xml
	-CMOCKA_XML_FILE=build/report/$(basename $(notdir $<)).xml CMOCKA_MESSAGE_OUTPUT=xml $< 
	-@lcov -q -c -d build/test -o $@ -t $(basename $(notdir $<))

coverage: tests $(TESTS_COV)
	-@lcov -t coverage $(addprefix -a ,$(TESTS_COV)) -o build/coverage.info
	-@genhtml -q -s -k --legend --highlight --output-directory build/coverage build/coverage.info
	#-@genhtml -q -f -s -k --legend --highlight --output-directory build/coverage build/coverage.info
	@ls -1 build/report/test*.xml | gawk 'BEGIN {print "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<files sysinfo=\"$(SYSINFO)\" created=\"" strftime("%Y-%m-%dT%H:%M:%S") "\">"} {n=split($$0,a,/\//); print "<file>" a[n] "</file>" } END {print "</files>\n"}' > build/report/report.xml
	@xsltproc --nonet -o build/report/index.html test/report.xsl build/report/report.xml


.PHONY: all tests check coverage clean

.PRECIOUS: build/test/%.t build/test/%.o build/%.o build/test/unit_%.o
