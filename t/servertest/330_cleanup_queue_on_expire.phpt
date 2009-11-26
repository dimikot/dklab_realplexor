--TEST--
dklab_realplexor: rotate queue data

--FILE--
<?php
$REALPLEXOR_CONF = "small_cleanup_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_in("identifier=abc", "
	a
");
send_in(null, "stats");
expect('/data_to_send=0/');
send_in(null, "stats");


?>
--EXPECT--
IN <== X-Realplexor: identifier=abc
IN <== 
IN <== "a"
IN ==> 
IN <== stats
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 138
IN ==> 
IN ==> [data_to_send]
IN ==> abc => [*: 3b]
IN ==> 
IN ==> [connected_fhs]
IN ==> 
IN ==> [online_timers]
IN ==> 
IN ==> [cleanup_timers]
IN ==> abc => assigned
IN ==> 
IN ==> [pairs_by_fhs]
IN <== stats
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 83
IN ==> 
IN ==> [data_to_send]
IN ==> 
IN ==> [connected_fhs]
IN ==> 
IN ==> [online_timers]
IN ==> 
IN ==> [cleanup_timers]
IN ==> 
IN ==> [pairs_by_fhs]
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]