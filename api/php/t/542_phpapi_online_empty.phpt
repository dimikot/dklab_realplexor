--TEST--
dklab_realplexor: PHP API test, online IDs (empty list)

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

printr($mpl->cmdOnline());

?>
--EXPECT--
array (
)
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]