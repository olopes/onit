#!/bin/bash

TCC_DEST=`pwd`/tcc
echo $TCC_DEST

#require cmocka https://cmocka.org/
mkdir -p build
pushd build

#curl http://download-mirror.savannah.gnu.org/releases/tinycc/tcc-0.9.27.tar.bz2 --output tcc-0.9.27.tar.bz2

tar xf tcc-0.9.27.tar.bz2 
cd tcc-0.9.27/
./configure --prefix=$TCC_DEST 
make
make install
make clean
./configure --cc=$TCC_DEST/bin/tcc --enable-static "--extra-cflags=-nostdlib -nostdinc" --prefix=$TCC_DEST
make
make install
popd
