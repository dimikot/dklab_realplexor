#!/bin/bash

# 
# Build & install GCC pre-requisities:
#   gmp, mpc, mprf
#
# Build & install GCC 4.6:
#   echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf && ldconfig
#   ./configure --with-mpc=/usr/local --with-gmp=/usr/local --with-mpfr=/usr/local --enable-languages=c,c++ --without-ppl --without-cloog
#
# Now you have the new GCC installed.
#
# Build & install Boost with C++0x support:
#   ./bootstrap.sh
#   ./bjam --toolset=gcc --cxxflags=-std=gnu++0x stage
#   ./bjam --toolset=gcc --cxxflags=-std=c++0x --build-type=complete --layout=tagged stage
#   ./bjam -a architecture=x86 instruction-set=i686 toolset=gcc cxxflags=-std=gnu++0x build-type=complete
#
# Build libev:
#   (cd libev && make)
#
# Install libmemcached:
#   yum install libmemcached-devel
#

GCC="g++ -std=gnu++0x -static"
#GCC="g++ -std=gnu++0x"
#DEBUG="-g3 -O0"
DEBUG="-O3"
export INCLUDE=$INCLUDE:libev
export LIB=$INCLUDE:libev/.libs

rm -f ../../dklab_realplexor 2>/dev/null
$GCC dklab_realplexor.cpp \
    $DEBUG -Wfatal-errors -Wall -Werror \
    -pthread -lcrypt -lboost_filesystem -lboost_system -lboost_regex -lev \
    -o ../../dklab_realplexor
exit $?
