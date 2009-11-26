<?php
require_once "_common.php";

try {
	if (!empty($_POST['ids'])) { 
		// Send data to the specified number of IDs.
		$ids = explode(",", $_POST['ids']);
		for ($i = 0; $i < max(@$_POST['repeat'], 1); $i++) {
			$mpl->send($ids, $_POST['message'] . (@$_POST['repeat'] > 1? " #$i" : ""));
		}
	}
} catch (Exception $e) {
	echo $e->getMessage();
}
