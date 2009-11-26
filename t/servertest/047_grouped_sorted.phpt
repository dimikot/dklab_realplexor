--TEST--
dklab_realplexor: multiple items with greater cursor with result ordering

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_in("identifier=20:abc", "
	ccc
");

send_in("identifier=11:abc", "
	aaa
");

send_in("identifier=18:abc", "
	ddd
");

send_in("identifier=15:abc", "
	bbb
");

send_wait("
	identifier=10:abc
");
recv_wait();


?>
--EXPECT--
IN <== X-Realplexor: identifier=20:abc
IN <== 
IN <== "ccc"
IN ==> 
IN <== X-Realplexor: identifier=11:abc
IN <== 
IN <== "aaa"
IN ==> 
IN <== X-Realplexor: identifier=18:abc
IN <== 
IN <== "ddd"
IN ==> 
IN <== X-Realplexor: identifier=15:abc
IN <== 
IN <== "bbb"
IN ==> 
WA <-- identifier=10:abc
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "11" },
WA -->     "data": "aaa"
WA -->   },
WA -->   {
WA -->     "ids": { "abc": "15" },
WA -->     "data": "bbb"
WA -->   },
WA -->   {
WA -->     "ids": { "abc": "18" },
WA -->     "data": "ddd"
WA -->   },
WA -->   {
WA -->     "ids": { "abc": "20" },
WA -->     "data": "ccc"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]