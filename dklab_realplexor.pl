#!/usr/bin/perl -w
#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

##
## This is a Perl version of dklab_realplexor, it is quite slow, but available
## by default. To build a binary version from C++ sources, see cpp/ directory.
##

use File::Basename;
use Cwd 'abs_path';
do(dirname(abs_path(__FILE__)) . "/perl/dklab_realplexor.pl");
die $@ if $@;
