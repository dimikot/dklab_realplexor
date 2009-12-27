--TEST--
dklab_realplexor: send with login: watch

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=10:user_abc
	aaa
");
disconnect_wait();

echo "Must be FAKE:\n";
send_in(null, "identifier=user:password@\n\n" . "watch");
echo "Must be FAKE:\n";
send_in(null, "identifier=user:password@\n\n" . "watch 1 u");
echo "Must be not empty:\n";
send_in(null, "identifier=user:password@\n\n" . "watch 1 user_");

?>
--EXPECT--
WA <-- identifier=10:user_abc
WA <-- aaa
WA :: Disconnecting.
Must be FAKE:
IN <== identifier=user:password@
IN <== 
IN <== watch
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 13
IN ==> 
IN ==> FAKE *:FAKE
Must be FAKE:
IN <== identifier=user:password@
IN <== 
IN <== watch 1 u
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 0
Must be not empty:
IN <== identifier=user:password@
IN <== 
IN <== watch 1 user_
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 19
IN ==> 
IN ==> online *:user_abc
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=1 cleanup_timers=0 events=*]