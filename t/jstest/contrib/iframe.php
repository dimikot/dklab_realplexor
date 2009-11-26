<?php
ini_set('display_errors', 1);
system('perl -we ' . escapeshellarg('
	BEGIN { chdir "../../.." }
	use Realplexor::Config;
	Realplexor::Config::load();
	print $CONFIG{IFRAME_CONTENT};
'));
