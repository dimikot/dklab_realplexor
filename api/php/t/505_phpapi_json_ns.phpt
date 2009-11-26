--TEST--
dklab_realplexor: PHP API test, JSON mode (with namespace)

--FILE--
<?php
$NAMESPACE = "ns_";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=ns_abc
	aaa
");

$mpl->send(array("abc", "def"), "Test!");

recv_wait();

?>
--EXPECT--
WA <-- identifier=ns_abc
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
WA -->     "ids": { "ns_abc": <cursor> },
WA -->     "data": "Test!"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=2 connected_fhs=0 online_timers=1 cleanup_timers=2 events=*]