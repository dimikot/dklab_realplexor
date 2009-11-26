--TEST--
dklab_realplexor: PHP API test, errors

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

try {
	$mpl = new Dklab_Realplexor("127.0.0.1", "10011");
	$mpl->send(array("abc"), "Test!");
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$mpl = new Dklab_Realplexor("127.0.0.1", "10011");
	$mpl->send(array("no-num"), "Test!");
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$mpl = new Dklab_Realplexor("127.0.0.1", "10011");
	$mpl->send(array("abc" => "aaa"), "Test!");
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
--EXPECT--
Error #111: Connection refused
Identifier must be alphanumeric, "no-num" given
Cursor must be numeric, "aaa" given
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]