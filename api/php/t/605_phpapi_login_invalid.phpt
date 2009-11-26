--TEST--
dklab_realplexor: PHP API test, login

--FILE--
<?php
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=user_abc
	aaa
");

$mpl->logon("unknown", "password");
try {
	$mpl->send(array("abc", "def"), "Test!");
} catch (Exception $e) {
	echo $e->getMessage();
}

?>
--EXPECT--
WA <-- identifier=user_abc
WA <-- aaa
Request failed: 403 Access Deined
unknown login: unknown
#   [pairs_by_fhs=1 data_to_send=0 connected_fhs=1 online_timers=1 cleanup_timers=0 events=*]