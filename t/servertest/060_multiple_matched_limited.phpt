--TEST--
dklab_realplexor: client reads only his subscribed IDs with limiter applied

--FILE--
<?php
require dirname(__FILE__) . '/init.php';

send_in("identifier=100:aa", "
	data to limiter ID
");

send_in("identifier=20:abc", "
	public data
");

send_in("identifier=20:abc,30:def,40:ghi,*aa,*bb", "
	limited data
");


echo "Only public data should be returned, matched 'abc'\n";
send_wait("
	identifier=10:abc,20:def
");
recv_wait();

echo "Return public and limited data, matched 'abc' and limiter 'aa'\n";
send_wait("
	identifier=10:abc,100:def,200:aa
");
recv_wait();


echo "Return limited data, matched 'def' and limiter 'bb'\n";
send_wait("
	identifier=100:abc,10:def,bb
");
recv_wait();


echo "Return data for limiter 'aa'\n";
send_wait("
	identifier=50:aa
");
recv_wait();


?>
--EXPECT--
IN <== X-Realplexor: identifier=100:aa
IN <== 
IN <== "data to limiter ID"
IN ==> 
IN <== X-Realplexor: identifier=20:abc
IN <== 
IN <== "public data"
IN ==> 
IN <== X-Realplexor: identifier=20:abc,30:def,40:ghi,*aa,*bb
IN <== 
IN <== "limited data"
IN ==> 
Only public data should be returned, matched 'abc'
WA <-- identifier=10:abc,20:def
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "20" },
WA -->     "data": "public data"
WA -->   }
WA --> ]
WA :: Disconnecting.
Return public and limited data, matched 'abc' and limiter 'aa'
WA <-- identifier=10:abc,100:def,200:aa
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": "20" },
WA -->     "data": "limited data"
WA -->   },
WA -->   {
WA -->     "ids": { "abc": "20" },
WA -->     "data": "public data"
WA -->   }
WA --> ]
WA :: Disconnecting.
Return limited data, matched 'def' and limiter 'bb'
WA <-- identifier=100:abc,10:def,bb
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "def": "30" },
WA -->     "data": "limited data"
WA -->   }
WA --> ]
WA :: Disconnecting.
Return data for limiter 'aa'
WA <-- identifier=50:aa
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "aa": "100" },
WA -->     "data": "data to limiter ID"
WA -->   }
WA --> ]
WA :: Disconnecting.
#   [pairs_by_fhs=0 data_to_send=4 connected_fhs=0 online_timers=4 cleanup_timers=4 events=*]