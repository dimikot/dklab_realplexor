--TEST--
dklab_realplexor: data cursor must be greater than listening cursor

--FILE--
<?php
$REALPLEXOR_CONF = "small_wait_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_in("identifier=5:abc", "
	aaa
");

send_wait("
	identifier=10:abc
");

send_in("identifier=6:abc", "
	aaa
");

recv_wait();


?>
--EXPECT--
IN <== X-Realplexor: identifier=5:abc
IN <== 
IN <== "aaa"
IN ==> 
WA <-- identifier=10:abc
IN <== X-Realplexor: identifier=6:abc
IN <== 
IN <== "aaa"
IN ==> 
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]