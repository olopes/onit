#!/bin/sh
../tcc/bin/tcc -o build/ccc test/test_main.c test/test_cmocka_examples.c ../cmocka/lib/cmocka.o 
