#!/bin/bash

#require cmocka https://cmocka.org/
mkdir -p build
pushd build
# wget https://cmocka.org/files/1.1/cmocka-1.1.3.tar.xz
curl https://cmocka.org/files/1.1/cmocka-1.1.3.tar.xz --output cmocka-1.1.3.tar.xz
tar xf cmocka-1.1.3.tar.xz
cd cmocka-1.1.3
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../../cmocka -DCMAKE_BUILD_TYPE=Debug ..
make && make install
cd ../..
rm -fr cmocka-1.1.3
rm -fr cmocka-1.1.3.tar.xz
cd ../cmocka
if [ -d "lib64" -a ! -e "lib" ]
then
	ln -s lib64 lib
fi
popd

