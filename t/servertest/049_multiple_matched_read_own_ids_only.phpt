--TEST--
dklab_realplexor: client reads only his subscribed IDs

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_in("identifier=20:abc,30:def,40:ghi", "
	ccc
");
send_wait("
	identifier=10:abc,20:def
");
recv_wait();


?>
--EXPECT--
IN <== X-Realplexor: identifier=20:abc,30:def,40:ghi
IN <== 
IN <== "ccc"
IN ==> 
WA <-- identifier=10:abc,20:def
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "20", "def": "30" },
WA -->     "data": "ccc"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=3 connected_fhs=0 online_timers=2 cleanup_timers=3 events=*]