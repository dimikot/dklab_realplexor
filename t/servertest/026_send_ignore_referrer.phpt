--TEST--
dklab_realplexor: ignore IDs in Referer header

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	Abc: def
	Referer: blabla?identifier=IFRAME
	identifier=abc,def
");
send_in("identifier=abc", "aaa");
recv_wait();

?>
--EXPECT--
WA <-- Abc: def
WA <-- Referer: blabla?identifier=IFRAME
WA <-- identifier=abc,def
IN <== X-Realplexor: identifier=abc
IN <== 
IN <== "aaa"
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
WA -->     "ids": { "abc": <cursor> },
WA -->     "data": "aaa"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=2 cleanup_timers=1 events=*]