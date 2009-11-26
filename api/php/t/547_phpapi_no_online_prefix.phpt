--TEST--
dklab_realplexor: PHP API test, online IDs with prefix, no onliners

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:A_abc,10:def,20:B_ghi
	aaa
");

printr($mpl->cmdOnline(array("xxx_")));

disconnect_wait();

?>
--EXPECT--
WA <-- identifier=5:A_abc,10:def,20:B_ghi
WA <-- aaa
array (
)
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=3 cleanup_timers=0 events=*]