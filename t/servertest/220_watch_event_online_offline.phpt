--TEST--
dklab_realplexor: watch online + offline status change

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=abc
	aaa
");
send_in(null, "watch 1");
disconnect_wait();

send_wait("
	identifier=def
	aaa
");
send_in(null, "watch 1");
disconnect_wait();

expect('/def.*offline/');

send_in(null, "watch 1");

?>
--EXPECT--
WA <-- identifier=abc
WA <-- aaa
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 14
IN ==> 
IN ==> online *:abc
WA :: Disconnecting.
WA <-- identifier=def
WA <-- aaa
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 28
IN ==> 
IN ==> online *:abc
IN ==> online *:def
WA :: Disconnecting.
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 30
IN ==> 
IN ==> offline *:abc
IN ==> offline *:def
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]