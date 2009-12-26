--TEST--
dklab_realplexor: send with login: data to not yet connected client

--FILE--
<?php
$VERBOSE = 1;
$REALPLEXOR_CONF = "non_anonymous.conf";
require dirname(__FILE__) . '/init.php';

send_in("identifier=user:password@user_abc", "
	aaa
");

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
IN <== X-Realplexor: identifier=user:password@user_abc
IN <== 
IN <== "aaa"
# IN: DEBUG: connection opened
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# IN: DEBUG: read <N> bytes
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# IN: DEBUG: parsed IDs; login is "user"
#   [pairs_by_fhs=0 data_to_send=0 connected_fhs=0 online_timers=0 cleanup_timers=0 events=*]
# IN: DEBUG: added data for [user_abc]
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=0 cleanup_timers=1 events=*]
# IN: DEBUG: connection closed
#   [pairs_by_fhs=0 data_to_send=1 connected_fhs=0 online_timers=0 cleanup_timers=1 events=*]
IN ==>