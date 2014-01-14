#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

Build dklab_realplexor binary on Ubuntu 12.04
---------------------------------------------

We recommend to build dklab_realplexor binary on Ubuntu 12.04. The resulting
binary will be portable: it depends on no shared libraries, so you may copy
it to any Linux distribution (even CentOS or RHEL) and use there.

For Ubuntu 12.04, the steps are simple:

# apt-get install gcc libboost1.48 libev4 libev-dev libmemcached-dev
# bash ./Make.sh


Build dklab_realplexor binary on other Linux versions
-----------------------------------------------------

If you have other version of Linux, please play the quest.

1.  Check your GCC version: "gcc --version"
    (run "apt-get install gcc" if needed).
1a. If the version is less than 4.6 in your system, install the newest one:
    a) Build & install GCC pre-requisities: gmp, mpc, mprf
    b) Build & install GCC 4.6:
       echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf && ldconfig
       ./configure --with-mpc=/usr/local --with-gmp=/usr/local --with-mpfr=/usr/local --enable-languages=c,c++ --without-ppl --without-cloog

2.  Install Boost: "apt-get install libboost1.48"
    (run "apt-cache pkgnames|grep libboost" to see exact Boost version in your system).
2a. If Boost is not available via your package manager, try to install Boost manually:
    a) Download and unpack Boost somewhere from http://www.boost.org/
    b) Build & install Boost:
       ./bootstrap.sh
       ./bjam --toolset=gcc --cxxflags=-std=gnu++0x stage
       ./bjam --toolset=gcc --cxxflags=-std=c++0x --build-type=complete --layout=tagged stage
       ./bjam -a architecture=x86 instruction-set=i686 toolset=gcc cxxflags=-std=gnu++0x build-type=complete

3.  Install libev: "apt-get install libev4 libev-dev"
3a. If not available, build libev manually: download, unpack and run  "(cd libev && make)"

4.  Install libmemcached: "apt-get install libmemcached-dev"
    (or "yum install libmemcached-devel" in RHEL).

5.  Run "bash ./Make.sh"
