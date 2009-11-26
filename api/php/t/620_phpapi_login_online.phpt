--TEST--
dklab_realplexor: PHP API test, login + online

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

$mpl->logon("user", "password");

send_wait("
	identifier=5:user_abc,10:user_def
	aaa
");

printr($mpl->cmdOnline());

disconnect_wait();

?>
--EXPECT--
WA <-- identifier=5:user_abc,10:user_def
WA <-- aaa
array (
  0 => 'abc',
  1 => 'def',
)
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=2 cleanup_timers=0 events=*]