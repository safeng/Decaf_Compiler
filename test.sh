#!/bin/bash

##** test.sh - A simple test framework ********************************

make clean
make
for x in samples/*.decaf
do
    ./dcc < $x &> $x.test
    if diff -urw $x.test ${x/.decaf/.out} > /dev/null
    then
        echo -e "\e[32m${x}\e[0m"
    else
        echo -e "\e[31m${x}\e[0m"
        diff -urw $x.test ${x/.decaf/.out}
    fi
done
rm samples/*.decaf.test
