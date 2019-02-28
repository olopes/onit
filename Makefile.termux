
all: build/onit build/lsh

build/onit:
	$(MAKE) -C onit all
	cp onit/build/onit build/onit

build/lsh:
	$(MAKE) -C lsh all
	cp lsh/build/lsh build/lsh

clean:
	rm -r -f -- build/onit build/lsh build/*.o build/test *.gcda *.gcno
	$(MAKE) -C onit clean
	$(MAKE) -C lsh clean
	
.PHONY: all clean

