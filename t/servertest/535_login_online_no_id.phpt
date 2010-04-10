--TEST--
dklab_realplexor: send with login: no id, only login + password

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=user_abc
	aaa
");
disconnect_wait();

send_in(null, "identifier=user:password@\n\n" . "online user_");

?>
--EXPECT--
WA <-- identifier=user_abc
WA <-- aaa
WA :: Disconnecting.
IN <== identifier=user:password@
IN <== 
IN <== online user_
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 11
IN ==> 
IN ==> user_abc 0
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=1 cleanup_timers=0 events=*]