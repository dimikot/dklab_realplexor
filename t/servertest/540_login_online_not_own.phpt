--TEST--
dklab_realplexor: send with login: online with not own prefix

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=user_abc
	aaa
");
disconnect_wait();

echo "Must be empty:\n";
send_in(null, "identifier=user:password@\n\n" . "online u");
echo "Must be empty:\n";
send_in(null, "identifier=user:password@\n\n" . "online u");
echo "Must be not empty:\n";
send_in(null, "identifier=user:password@\n\n" . "online user_");

?>
--EXPECT--
WA <-- identifier=user_abc
WA <-- aaa
WA :: Disconnecting.
Must be empty:
IN <== identifier=user:password@
IN <== 
IN <== online u
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 0
Must be empty:
IN <== identifier=user:password@
IN <== 
IN <== online u
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 0
Must be not empty:
IN <== identifier=user:password@
IN <== 
IN <== online user_
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 11
IN ==> 
IN ==> user_abc 0
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=1 cleanup_timers=0 events=*]