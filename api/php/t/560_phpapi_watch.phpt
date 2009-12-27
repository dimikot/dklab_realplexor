--TEST--
dklab_realplexor: PHP API test, command "watch"

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:abc
	aaa
");
disconnect_wait();
send_wait("
	identifier=6:def
	aaa
");
disconnect_wait();

$events = $mpl->cmdWatch(1);
foreach ($events as $i => $e) {
	$events[$i]['pos'] = "*";
}
printr($events);


?>
--EXPECT--
WA <-- identifier=5:abc
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=6:def
WA <-- aaa
WA :: Disconnecting.
array (
  0 => 
  array (
    'event' => 'online',
    'pos' => '*',
    'id' => 'abc',
  ),
  1 => 
  array (
    'event' => 'online',
    'pos' => '*',
    'id' => 'def',
  ),
)
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=2 cleanup_timers=0 events=*]