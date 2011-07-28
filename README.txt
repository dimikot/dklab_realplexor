Dklab Realplexor v1.40: Comet server which handles 1000000+ parallel browser connections.
Author: Dmitry Koterov, dkLab (C)
Home page: http://dklab.ru/lib/dklab_realplexor/
Changelog: http://github.com/DmitryKoterov/dklab_realplexor/commits/master/


INSTALLATION ON LINUX
---------------------

0. First of all, run ./dklab_realplexor.pl manually and check that all
   needed libraries are installed. If not, compile & install them:
   - For RHEL (RedHat, CentOS):
     # yum install gcc
     # perl -MCPAN -e "install EV"
   - For Debian (or Ubuntu):
     # apt-get install gcc
     # perl -MCPAN -e "install EV"

1. Copy Realplexor to /opt/dklab_realplexor (or you may create a symlink).
   # cp -a . /opt/dklab_realplexor
     - or -
   # ln -s `pwd` /opt/dklab_realplexor

2. Create /etc/dklab_realplexor.conf if you need a custom configuration.
   (You may create a symlink instead of creating the file.)

   # cat > /etc/dklab_realplexor.conf
   $CONFIG{WAIT_ADDR} = [ '1.2.3.4:80' ];  # your IP address and port
   $CONFIG{IN_ADDR} = [ '5.6.7.8:10010' ]; # for IN line
   return 1;
   ^D

     - or -

   # ln -s /path/to/your/config.conf /etc/dklab_realplexor.conf 
   
3. Use bundled init-script to start Realplexor as a Linux service:
   # ln -s /opt/dklab_realplexor/dklab_realplexor.init /etc/init.d/dklab_realplexor
   
4. Tell your system to start Realplexor at boot:
   - For RHEL (RedHat, CentOS):
     # chkconfig --add dklab_realplexor
     # chkconfig dklab_realplexor on
   - For Debian (or Ubuntu):
     # update-rc.d dklab_realplexor defaults
     # update-rc.d dklab_realplexor start


SYNOPSIS
--------

1. In JavaScript code, execute:
<script type="text/javascript" src="/path/to/dklab_realplexor.js"></script>
var realplexor = new Dklab_Realplexor("http://rpl.yoursite.com/");
realplexor.subscribe("alpha", function(data) { alert("alpha: " + data) });
realplexor.subscribe("beta", function(data) { alert("beta: " + data) });
realplexor.execute();

2. In PHP code, execute:
require dirname(__FILE__) . '/Dklab/Realplexor.php';
$realplexor = new Dklab_Realplexor("127.0.0.1", "10010");
$realplexor->send(array("alpha", "beta"), "hello!");

3. See more details in Realplexor documentation.


LOG MNEMONICS
-------------

pairs_by_fhs
  Number of active TCP connections on WAIT line (clients).

data_to_send
  Number of IDs with non-empty command queue.

connected_fhs
  Number of IDs which are listened by at least one client.
  
online_timers
  Number of "online" client identifiers. Client is treated as online if:
  - it has an active connection;
  - or it does not have a connection, but disconnected no more than
    OFFLINE_TIMEOUT seconds ago.

cleanup_timers
  Number of IDs which queue must be cleaned if no activity is present for
  a long time. This is a unused IDs garbage collector statistics.

events
  How many events (e.g. ONLINE/OFFLINE status changes) are collected
  by realplexor. Event queue is limited by size.


CHANGELOG
---------

* Dklab Realplexor 2011-07-28: v1.40
  - [NEW] Python API added (experimental)
  - [NEW] Added missed "return this" for chained JS calls.
  - [BUG] Firefox 4 bugfix against 'attempt to run compile-and-go script on a cleared scope'.
  - [BUG] Minor changes & better support for phpt tests.
  - [BUG] No OFFLINE event should be generated until the last connection with ID is disconnected.
  - [BUG] If no IDs are subscribed in JS, do not connect to the server with empty ID list.

* Dklab Realplexor 2010-08-11: v1.32
  - [SPD] When empty HTTP body is passed to IN connection, it is now ignored, no warnings generated.
  - [SPD] Remove old data from channels BEFORE data processing/sending.
  - [BUG] Use print instead of syswrite, because for large amount of data syswrite sometimes 
    returns before all this data is transmitted.

* Dklab Realplexor 2010-04-16: v1.31
  - [BUG] Perl does not call flush() automatically before socket shutdown(). It 
    sometimes (unstable!) causes unexpected SIGPIPEs and data loss. Fixed: now flush() 
    is called manually.
  - [BUG] STATS command is not processed twice anymore.
  - [NEW] Ability to limit memory usage and auto-restart the daemon if it 
    consumes too much memory. (Note that unsent data is lost during this restart.)
  - [NEW] PHP API: cmdOnlineWithCounters(): for each online ID also returns
    the number of browsers connected just now (it is NOT a "number of online
    users who listen this channel", but its approximation).
  - [BUG] Minor fixes in clean_old_data_for_id (bug is not reproduced, 
    but now surrounding code is better).
  - [NEW] Visibility:hidden for IFRAME. It is good when BODY has relative position.
  - [BUG] Allow to pass a scalar to 2nd parameter of cmdWatch($fromPos, $idPrefixes).

* Dklab Realplexor 2010-02-27: v1.30
  - [SPD] Use EV library (http://search.cpan.org/~mlehmann/EV-3.9/EV.pm)
    instead of libevent. It is faster and has no memory leaks.

* Dklab Realplexor 2010-01-30: v1.24
  - [BUG] Avoid warnings in log on unexpected disconnect.
  - [NEW] Refactoring and profiler support.
  - [SPD] Do not create extra shell while calling ulimit.
  - [NEW] Support for per-config log facility.
  - [SPD] Profiler tool with IN line ignorance. Avoid BigFloat in events: 45% speedup. Apache ab patched utility.
  - [SPD] Keep channels pre-sorted after addition. It speedups 60%, because we need less cursor comparisions.
  - [SPD] STDOUT buffering in non-verbose mode. More verbosity levels. Logger speedup. Custom config for profiler script.

* Dklab Realplexor 2009-12-26: v1.23
  - [BUG] Empty identifier passed to IN line ("identifier=") caused warnings.
  - [SPD] Lower the number of useless debug lines and connection's name() calls.
  - [BUG] Improved init script: more time to restart and better signal handling.

* Dklab Realplexor 2009-12-24: v1.22
  - [BUG] SIGPIPE causes the script to restart on some unexpected client's disconnects.

* Dklab Realplexor 2009-12-22: v1.21
  - [NEW] ID queue is cleaned after CLEAN_ID_AFTER seconds when no data arrived
    (previously OFFLINE_TIMEOUT was used for that).
  - [NEW] To unsubscribe all callbacks from a channel: rpl.unsubscribe("channel", null).

* Dklab Realplexor 2009-12-16: v1.15
  - [NEW] When IDs list is long, JS API uses POST request instead of GET.
  - [NEW] IN line now fully supports HTTP POST.
  - [NEW] Non-200 responses from IN line are converted to exceptions.
  - [NEW] Content-Length verification in PHP API.
  - [NEW] Support for SSL in IN line for PHP API (use 443 port).
  - [BUG] If callback called execute(), extra request was performed.
  - [BUG] Referrer header was not ignored by server engine (bad if it contains IFRAME marker).
