#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections.
#@ Author: Dmitry Koterov, dkLab (C)
#@ Home page: http://dklab.ru/lib/dklab_realplexor/
#@ Changelog: http://github.com/DmitryKoterov/dklab_realplexor/commits/master/
#@

Note that these crash test are bad, because a bottleneck
is not a Realplexor, but a load-test script itself. Seems
Perl works with multiple connections in a non-event-driven
environment too inefficiently.
