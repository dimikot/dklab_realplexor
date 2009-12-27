--TEST--
dklab_realplexor: PHP API test, command "watch" with namespace and prefixes

--FILE--
<?php
$NAMESPACE = "ns_";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=5:ns_demo_abc
	aaa
");
disconnect_wait();
send_wait("
	identifier=6:def
	aaa
");
disconnect_wait();
send_wait("
	identifier=7:ns_ghi
	aaa
");
disconnect_wait();

$events = $mpl->cmdWatch(1, array("demo_"));
foreach ($events as $i => $e) {
	$events[$i]['pos'] = "*";
}
printr($events);


?>
--EXPECT--
WA <-- identifier=5:ns_demo_abc
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=6:def
WA <-- aaa
WA :: Disconnecting.
WA <-- identifier=7:ns_ghi
WA <-- aaa
WA :: Disconnecting.
array (
  0 => 
  array (
    'event' => 'online',
    'pos' => '*',
    'id' => 'demo_abc',
  ),
)
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=3 cleanup_timers=0 events=*]