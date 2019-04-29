#!/bin/sh

BASE_PATH=`dirname "${BASH_SOURCE[0]}"`
BASE_PATH=`( cd "$BASE_PATH/.." && pwd )`
# echo "$MY_PATH"
export TCC_HOME=${BASE_PATH}/tcc
export PATH=$TCC_HOME/bin:$PATH

