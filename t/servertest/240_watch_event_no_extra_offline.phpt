--TEST--
dklab_realplexor: no OFFLINE event should be generated until the last ID is disconnected

--FILE--
<?php
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

echo "\n## First client connected and had not disconnected.\n";
send_wait("
	identifier=1:abc
	aaa
");
$socket1 = $WAIT_SOCK;

echo "\n## Second client with the same ID connected+disconnected after 2 seconds.\n";
send_wait("
	identifier=2:abc
	bbb
");
disconnect_wait();

echo "\n## In 3 seconds ID should still be online because of the first active connection.\n";
sleep(3);
send_in(null, "watch 1");

echo "\n## Now we disconnect the first connection.\n";
$WAIT_SOCK = $socket1; $socket1 = null;
disconnect_wait();

echo "\n## No OFFLINE should be within 0.2 seconds.\n";
usleep(200000);
send_in(null, "watch 1");

echo "\n## In 2 seconds we should get OFFLINE message.\n";
sleep(2);
send_in(null, "watch 1");

?>
--EXPECT--
## First client connected and had not disconnected.
WA <-- identifier=1:abc
WA <-- aaa

## Second client with the same ID connected+disconnected after 2 seconds.
WA <-- identifier=2:abc
WA <-- bbb
WA :: Disconnecting.

## In 3 seconds ID should still be online because of the first active connection.
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 14
IN ==> 
IN ==> online *:abc

## Now we disconnect the first connection.
WA :: Disconnecting.

## No OFFLINE should be within 0.2 seconds.
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 14
IN ==> 
IN ==> online *:abc

## In 2 seconds we should get OFFLINE message.
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 15
IN ==> 
IN ==> offline *:abc
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]