--TEST--
dklab_realplexor: PHP API test, online IDs with online browsers counters

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:abc,10:def,6:abc
	aaa
");

printr($mpl->cmdOnlineWithCounters());

disconnect_wait();

?>
--EXPECT--
WA <-- identifier=5:abc,10:def,6:abc
WA <-- aaa
array (
  'abc' => '1',
  'def' => '1',
)
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=2 cleanup_timers=0 events=*]