--TEST--
dklab_realplexor: initial WATCH command

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_in(null, "watch ?");

send_wait("
	identifier=abc
	aaa
");
disconnect_wait();
send_in(null, "watch ?");


?>
--EXPECT--
IN <== watch ?
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 13
IN ==> 
IN ==> FAKE *:FAKE
WA <-- identifier=abc
WA <-- aaa
WA :: Disconnecting.
IN <== watch ?
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 13
IN ==> 
IN ==> FAKE *:FAKE
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=1 cleanup_timers=0 events=*]