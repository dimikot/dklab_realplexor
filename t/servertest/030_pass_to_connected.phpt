--TEST--
dklab_realplexor: data is passed to connected client

--FILE--
<?php
$VERBOSE = 1;
$REALPLEXOR_CONF = "small_offline_timeout.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=abc
");
send_in("identifier=abc", "
	aaa
");
recv_wait();
expect('/is now offline/');

?>
--EXPECT--
# Starting.
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# CONFIG: appending configuration from ***
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: listening 0.0.0.0:8088
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# IN: listening 127.0.0.1:10010
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# Switching current user to unprivileged "nobody"
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
WA <-- identifier=abc
# WAIT: DEBUG: connection opened
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: DEBUG: read <N> bytes
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: DEBUG: [*:abc] registered
#   [pairs_by_fhs=1 data_to_send=0 connected_fhs=1 online_timers=1 cleanup_timers=0 events=*]
IN <== X-Realplexor: identifier=abc
IN <== 
IN <== "aaa"
# IN: DEBUG: connection opened
#   [pairs_by_fhs=1 data_to_send=0 connected_fhs=1 online_timers=1 cleanup_timers=0 events=*]
# IN: DEBUG: read <N> bytes
#   [pairs_by_fhs=1 data_to_send=0 connected_fhs=1 online_timers=1 cleanup_timers=0 events=*]
# IN: DEBUG: parsed IDs
#   [pairs_by_fhs=1 data_to_send=0 connected_fhs=1 online_timers=1 cleanup_timers=0 events=*]
# IN: DEBUG: added data for [abc]
#   [pairs_by_fhs=1 data_to_send=1 connected_fhs=1 online_timers=1 cleanup_timers=1 events=*]
# <- sending 1 responses (<N> bytes) from [abc] (print=1, shutdown=1)
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]
# IN: DEBUG: connection closed
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]
# WAIT: DEBUG: [*:abc] connection closed
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=1 cleanup_timers=1 events=*]
IN ==> 
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Cache-Control: no-store, no-cache, must-revalidate
WA --> Expires: ***
WA --> Content-Type: text/javascript; charset=utf-8
WA --> 
WA -->  
WA --> [
WA -->   {
WA -->     "ids": { "abc": <cursor> },
WA -->     "data": "aaa"
WA -->   }
WA --> ]
WA :: Disconnecting.
# [abc] is now offline
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=0 cleanup_timers=1 events=*]