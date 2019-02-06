#!/bin/sh

#require cmocka https://cmocka.org/
cmake -DCMAKE_INSTALL_PREFIX=`pwd`/../../cmocka -DCMAKE_BUILD_TYPE=Debug ..

