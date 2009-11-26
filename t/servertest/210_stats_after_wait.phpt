--TEST--
dklab_realplexor: stats after WA client connected + disconnected is OK

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=abc
	aaa
");

send_in(null, "stats\n");

disconnect_wait();

send_in(null, "stats\n");

expect('/is now offline/');

send_in(null, "stats\n");

?>
--EXPECT--
WA <-- identifier=abc
WA <-- aaa
IN <== stats
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 163
IN ==> 
IN ==> [data_to_send]
IN ==> 
IN ==> [connected_fhs]
IN ==> abc => (0x*)
IN ==> 
IN ==> [online_timers]
IN ==> abc => assigned
IN ==> 
IN ==> [cleanup_timers]
IN ==> 
IN ==> [pairs_by_fhs]
IN ==> (0x*) => *:abc
WA :: Disconnecting.
IN <== stats
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 99
IN ==> 
IN ==> [data_to_send]
IN ==> 
IN ==> [connected_fhs]
IN ==> 
IN ==> [online_timers]
IN ==> abc => assigned
IN ==> 
IN ==> [cleanup_timers]
IN ==> 
IN ==> [pairs_by_fhs]
IN <== stats
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 83
IN ==> 
IN ==> [data_to_send]
IN ==> 
IN ==> [connected_fhs]
IN ==> 
IN ==> [online_timers]
IN ==> 
IN ==> [cleanup_timers]
IN ==> 
IN ==> [pairs_by_fhs]
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]