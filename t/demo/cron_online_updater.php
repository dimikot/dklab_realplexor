<?php
ini_set("display_errors", 1);
require_once "_common.php";

while (1) {
	try {
		echo "Fetching initial statuses...\n";
		$online = array_flip($mpl->cmdOnline());
		echo "Fetched initial " . count($online) . " statuses\n";
		$mpl->send("who_is_online", array_keys($online));

		$pos = 0;
		while (1) {
			echo "Watching from $pos...\n";
			$read = false;
			foreach ($mpl->cmdWatch($pos) as $event) {
				echo "[" . date("r") . "] Received: {$event['event']} - {$event['id']}\n";
				if ($event['event'] == "offline") {
					unset($online[$event['id']]);
				} else if ($event['event'] == "online") {
					$online[$event['id']] = true;
				}
				$pos = $event['pos'];
				$read = true;
			}
			if ($read) {
				$mpl->send("who_is_online", array_keys($online));
			}
			sleep(1);
		}
	} catch (Exception $e) {
		echo "Exception: {$e->getMessage()}\n";
	}
	sleep(1);
}
