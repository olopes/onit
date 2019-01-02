
CFLAGS=-g -ansi -std=c99 -Wall -pedantic -finput-charset=UTF-8
LDFLAGS=-static 

all: build/onit build/ostr build/sexpr
# all: run-tests build/onit

build/onit: src/onit.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

build/ostr: src/ostr.c src/ostr_main.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

build/sexpr: src/sexpr.c src/sexpr_main.c src/sexpr_stack.c src/sobj.c src/svisitor.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -f build/sexpr build/onit build/ostr build/*.o

run-tests: build/test1 bin/test2 bin/test3
	build/test1
	build/test2
	build/test3

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
