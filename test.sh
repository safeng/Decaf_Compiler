#!/bin/bash

##** test.sh - A simple test framework ********************************

make clean
if make
then
    for x in samples/*.decaf
    do
        ./solution/dcc < $x &> ${x/.decaf/.out}
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
fi
