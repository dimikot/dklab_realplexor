--TEST--
dklab_realplexor: PHP API test, command "watch" if no events yet exist

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

printr($mpl->cmdWatch(0));

?>
--EXPECT--
array (
  0 => 
  array (
    'event' => 'FAKE',
    'pos' => '10',
    'id' => 'FAKE',
  ),
)
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]