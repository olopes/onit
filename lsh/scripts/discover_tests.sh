#!/bin/bash

unit_tests=`sed -n -E 's/((U)nitTest|(B)eforeAll|(A)fterAll)\(([_[:alnum:]]+)[^[:alnum:]]*/\2\3\4\5/p'`
echo '/* generated automatically. do not modify.*/'
echo '#include "test_definitions.h"'
echo 

for ut in $unit_tests
do
    case $ut in 
    [AB]*)
        echo "extern struct GroupSetupTeardown _${ut#?};"
        ;;
    U*)
        echo "extern struct LshUnitTest _${ut#?};"
        ;;
    esac
done
echo 

echo 'struct LshUnitTest* test_list[] = {'
for ut in $unit_tests
do
    case $ut in 
    U*)
        echo "  &_${ut#?},"
        ;;
    esac
done
echo '  NULL'
echo '};'
echo 


echo 'struct GroupSetupTeardown* test_conf[] = {'
for ut in $unit_tests
do
    case $ut in 
    [AB]*)
        echo "  &_${ut#?},"
        ;;
    esac
done
echo '  NULL'
echo '};'
echo 
