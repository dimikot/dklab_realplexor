--TEST--
dklab_realplexor: PHP API test, online IDs

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:abc,10:def
	aaa
");

printr($mpl->cmdOnline());

disconnect_wait();

?>
--EXPECT--
WA <-- identifier=5:abc,10:def
WA <-- aaa
array (
  0 => 'abc',
  1 => 'def',
)
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=2 cleanup_timers=0 events=*]