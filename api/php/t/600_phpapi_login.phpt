--TEST--
dklab_realplexor: PHP API test, login

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=user_abc
	aaa
");

$mpl->logon("user", "password");
$mpl->send(array("abc", "def"), "Test!");

recv_wait();

?>
--EXPECT--
WA <-- identifier=user_abc
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
WA -->     "ids": { "user_abc": <cursor> },
WA -->     "data": "Test!"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=2 connected_fhs=0 online_timers=1 cleanup_timers=2 events=*]