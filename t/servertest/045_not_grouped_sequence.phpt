--TEST--
dklab_realplexor: pass 2 data items with greater cursor (not grouped)

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=10:abc
");

send_in("identifier=15:abc", "
	aaa
");

send_in("identifier=20:abc", "
	bbb
");

recv_wait();

send_wait("
	identifier=15:abc
");

recv_wait();


?>
--EXPECT--
WA <-- identifier=10:abc
IN <== X-Realplexor: identifier=15:abc
IN <== 
IN <== "aaa"
IN ==> 
IN <== X-Realplexor: identifier=20:abc
IN <== 
IN <== "bbb"
IN ==> 
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "15" },
WA -->     "data": "aaa"
WA -->   }
WA --> ]
WA :: Disconnecting.
WA <-- identifier=15:abc
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "20" },
WA -->     "data": "bbb"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]