--TEST--
dklab_realplexor: data to multiple disconnected but online client is buffered

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_in("identifier=2:abc,2:def", "
	aaa
");
send_in("identifier=2:abc,2:def", "
	bbb
");

send_wait("
	identifier=1:abc,1:def
");
recv_wait();
expect('/is now offline/');

?>
--EXPECT--
IN <== X-Realplexor: identifier=2:abc,2:def
IN <== 
IN <== "aaa"
IN ==> 
IN <== X-Realplexor: identifier=2:abc,2:def
IN <== 
IN <== "bbb"
IN ==> 
WA <-- identifier=1:abc,1:def
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "2", "def": "2" },
WA -->     "data": "aaa"
WA -->   },
WA -->   {
WA -->     "ids": { "abc": "2", "def": "2" },
WA -->     "data": "bbb"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=2 connected_fhs=0 online_timers=0 cleanup_timers=2 events=*]