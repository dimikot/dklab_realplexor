--TEST--
dklab_realplexor: online cmd

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=abc
	aaa
");
disconnect_wait();

send_wait("
	identifier=def
	bbb
");

send_in(null, "online");

disconnect_wait();

?>
--EXPECT--
WA <-- identifier=abc
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=def
WA <-- bbb
IN <== online
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 12
IN ==> 
IN ==> abc 0
IN ==> def 1
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=2 cleanup_timers=0 events=*]