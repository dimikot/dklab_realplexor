--TEST--
dklab_realplexor: PHP API test, JSON mode (with namespace and limited IDs)

--FILE--
<?php
$NAMESPACE = "ns_";
require dirname(__FILE__) . '/init.php';

$mpl->send(array("abc" => 10, "def" => 10), "public data");
$mpl->send(array("abc" => 11, "def" => 11), "private data", array("myid1", "myid2"));


echo "Prints only public data\n";
send_wait("
	identifier=5:ns_abc
	aaa
");
recv_wait();


echo "Prints public and private data\n";
send_wait("
	identifier=5:ns_abc,666:ns_myid1
	aaa
");
recv_wait();



?>
--EXPECT--
Prints only public data
WA <-- identifier=5:ns_abc
WA <-- aaa
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "ns_abc": "10" },
WA -->     "data": "public data"
WA -->   }
WA --> ]
WA :: Disconnecting.
Prints public and private data
WA <-- identifier=5:ns_abc,666:ns_myid1
WA <-- aaa
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "ns_abc": "10" },
WA -->     "data": "public data"
WA -->   },
WA -->   {
WA -->     "ids": { "ns_abc": "11" },
WA -->     "data": "private data"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=2 connected_fhs=0 online_timers=2 cleanup_timers=2 events=*]