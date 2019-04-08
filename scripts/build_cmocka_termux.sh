#!/bin/bash

#require cmocka https://cmocka.org/
mkdir -p build
pushd build
# wget https://cmocka.org/files/1.1/cmocka-1.1.3.tar.xz
curl https://cmocka.org/files/1.1/cmocka-1.1.5.tar.xz --output cmocka-1.1.5.tar.xz
tar xf cmocka-1.1.5.tar.xz
cd cmocka-1.1.5

# ../../../tcc/bin/tcc -c cmocka.c -o ../../../cmocka/lib/cmocka-linux.o -static -fPIC -g -I. -I../include -DHAVE_SIGNAL_H
# gcc -c -static -fPIC -g -o ../../cmocka/lib/cmocka-linux-gcc.o src/cmocka.c -Isrc -Iinclude -DHAVE_SIGNAL_H
# ../../tcc/bin/tcc -c -static -fPIC -g -o ../../cmocka/lib/cmocka-linux-tcc.o src/cmocka.c -Isrc -Iinclude -DHAVE_SIGNAL_H
mkdir -p ../../cmocka/lib
mkdir -p ../../cmocka/include
gcc -c -fPIC -g -o ../../cmocka/lib/cmocka.o src/cmocka.c -Isrc -Iinclude
cp include/cmocka.h ../../cmocka/include
cp include/cmocka_pbc.h ../../cmocka/include

popd


