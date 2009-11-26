--TEST--
dklab_realplexor: watch event when no events yet exist

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_in(null, "watch 1");

?>
--EXPECT--
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 0
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]