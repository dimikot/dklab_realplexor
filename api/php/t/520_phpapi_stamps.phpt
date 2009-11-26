--TEST--
dklab_realplexor: PHP API test, pass cursors

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:abc
	aaa
");

$mpl->send(array("abc" => 10, "def" => 20, "ghi"), "Test!");

recv_wait();

?>
--EXPECT--
WA <-- identifier=5:abc
WA <-- aaa
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "10" },
WA -->     "data": "Test!"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=3 connected_fhs=0 online_timers=1 cleanup_timers=3 events=*]