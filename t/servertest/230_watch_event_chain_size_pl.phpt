--TEST--
dklab_realplexor: check chains usage

--SKIPIF--
<?php 
$NORUN = 1;
require dirname(__FILE__) . '/init.php';
if ($IS_BIN) echo 'skip'; 
?>
--FILE--
<?php
$REALPLEXOR_CONF = "small_chain_len.conf";
require dirname(__FILE__) . '/init.php';
for ($i = 0; $i < 20; $i++) {
	send_wait("
		identifier=abc" . sprintf("%02d", $i) . "
		aaa
	");
	disconnect_wait();
}

send_in(null, "watch 1");

?>
--EXPECT--
WA <-- identifier=abc00
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc01
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc02
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc03
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc04
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc05
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc06
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc07
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc08
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc09
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc10
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc11
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc12
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc13
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc14
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc15
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc16
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc17
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc18
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=abc19
WA <-- aaa
WA :: Disconnecting.
IN <== watch 1
IN ==> HTTP/1.0 200 OK
IN ==> Content-Type: text/plain
IN ==> Content-Length: 192
IN ==> 
IN ==> online *:abc08
IN ==> online *:abc09
IN ==> online *:abc10
IN ==> online *:abc11
IN ==> online *:abc12
IN ==> online *:abc13
IN ==> online *:abc14
IN ==> online *:abc15
IN ==> online *:abc16
IN ==> online *:abc17
IN ==> online *:abc18
IN ==> online *:abc19
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=20 cleanup_timers=0 events=*]