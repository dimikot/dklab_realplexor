--TEST--
dklab_realplexor: generate SCRIPT response

--FILE--
<?php
$VERBOSE = 1;
$REALPLEXOR_CONF = "script_stub.conf";
require dirname(__FILE__) . '/init.php';

send_wait("
	identifier=SCRIPT
	aaa
");
recv_wait();

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
WA <-- identifier=SCRIPT
WA <-- aaa
# WAIT: DEBUG: connection opened
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: DEBUG: read <N> bytes
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: DEBUG: SCRIPT marker received, sending content
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# WAIT: DEBUG: connection closed
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
WA --> HTTP/1.1 200 OK
WA --> Connection: close
WA --> Content-Type: text/javascript; charset=utf-8
WA --> Last-Modified: ***
WA --> Expires: ***
WA --> Cache-Control: public
WA --> 
WA --> SCRIPT stub: [SCRIPT].
WA :: Disconnecting.
