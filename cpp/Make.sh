#!/bin/bash

GCC="g++ -std=gnu++0x -static"
#GCC="g++ -std=gnu++0x"
#DEBUG="-g3 -O0"
DEBUG="-O3"
export INCLUDE=$INCLUDE:libev
export LIB=$INCLUDE:libev/.libs

rm -f ../dklab_realplexor 2>/dev/null
$GCC dklab_realplexor.cpp \
    $DEBUG -Wfatal-errors -Wall -Werror \
    -pthread -lcrypt -lboost_filesystem -lboost_system -lboost_regex -lev \
    -o ../dklab_realplexor
exit $?
