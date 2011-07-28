<?php
chdir(dirname(__FILE__));
ini_set('display_errors', true);
error_reporting(E_ALL);
require dirname(__FILE__) . '/../../api/php/Dklab/Realplexor.php';

// Values:
// 0: turn off debug info
// 1: show realplexor log
// 2: show system() calls log
if (!isset($VERBOSE)) $VERBOSE = @$_SERVER['argv'][1] == "-v"? 1 : 0;

// Arguments to override default config.
if (!isset($REALPLEXOR_CONF)) $REALPLEXOR_CONF = '';

// Testing a binary version?
$IS_BIN = strlen(getenv("isbin"))? !!getenv("isbin") : @is_executable(dirname(__FILE__) . "/../../dklab_realplexor");

// Debug mode (no output filtering)?
// ATTENTION: must NOT be set to 1 if VERBOSE is active! Verbose is
// used within unit-tests too, so it is NOT a fully-debug flag.
$DEBUG = !!getenv('debug') || (@$_SERVER['argv'][1] == "-v");

// Do not run the daemon?
$NORUN = !!@$NORUN;


// Start the realplexor.
if (empty($NORUN)) {
	start_realplexor();
	register_shutdown_function('kill_realplexor'); # MUST be AFTER starting due to fork()!
}

// Debug human-readable output of any variable.
function printr($value, $comment=null)
{
	if ($comment !== null) echo "$comment: ";
	var_export($value);
	echo "\n";
}

function run($cmd)
{
	if ($GLOBALS['VERBOSE'] > 1) echo "# $cmd\n";
	system($cmd);
}

function kill_realplexor()
{
	global $OUT_TMP;
	run("ps ax | perl -ne '/^\s*(\d+).*dklab_[r]ealplexor/ and kill 9, $1'");
	if ($OUT_TMP) unlink($OUT_TMP);
}

function start_realplexor()
{
	global $REALPLEXOR_CONF, $OUT_TMP, $OUT_TMP_FH, $IS_BIN, $DEBUG;
	kill_realplexor();
	$OUT_TMP = tempnam('non-existent', '');
	if (pcntl_fork() == 0) {
		if (!$DEBUG) {
			$filter = '
				$| = 1;
				s/\s*\[\w\w\w\s.*?\]\s*//sg;
				s/\s*Opened files limit.*//mg;
				s/\d+\.\d+\.\d+\.\d+:\d+:\s*//sg;
				s/\?:\?:\s*//sg;
				s/\d+( bytes)/<N>$1/s;
				s/(appending configuration from ).*/$1***/mg;
				s/(\[)\d+\.\d+/$1*/sg;
				s/(events=)\d+/$1*/sg;
				s/^/# /sg;
				if ($del) {
					$_ = "";
					$del--;
				}
			';
		} else {
			$filter = '$| = 1';
		}
		if ($IS_BIN) {
			$exe = "./dklab_realplexor";
			$conf = $REALPLEXOR_CONF;
		} else {
			$exe = "perl dklab_realplexor.pl";
			$conf = $REALPLEXOR_CONF;
		}
		$args = '';
		if ($conf) {
			$args = escapeshellarg(dirname(__FILE__) . '/fixture/' . $conf);
		}
		run("cd ../.. && $exe $args 2>&1 | tee -a $OUT_TMP " .
			"| perl -pe " . escapeshellarg($filter) .
			($GLOBALS['VERBOSE'] ? "" : " | tail -n1")
		);
		exit();
	}
	expect('/Switching current user/');
}

function expect($re)
{
	global $OUT_TMP, $OUT_TMP_POS;
	if (!$OUT_TMP_POS) $OUT_TMP_POS = 0;
	// Wait for the realplexor is started.
	if (@$_SERVER['argv'][1] == "-v") {
		echo "> expecting $re\n";
	}
	while (1) {
		$f = @fopen($OUT_TMP, "r");
		if ($f) {
			fseek($f, $OUT_TMP_POS, SEEK_SET);
			$line = fgets($f);
			fclose($f);
			$OUT_TMP_POS += strlen($line);
			if (preg_match($re, $line)) break;
		}
		usleep(10000);
	}
}

function send_in($ids, $data, $noWaitResponse = false)
{
	global $IN_SOCK;
	$data = trim(preg_replace('/^[ \t]+/m', '', $data));
	if ($ids !== null) {
		$out = "X-Realplexor: $ids\r\n\r\n";
		$out .= json_encode($data);
	} else {
		$out = $data;
	}
	echo preg_replace('/^/m', 'IN <== ', $out) . "\n";
	$IN_SOCK = fsockopen("127.0.0.1", 10010);
	fwrite($IN_SOCK, $out);
	if (!$noWaitResponse) {
		// Do not shutdown writing to the socket, else realplexor
		// destroys active connection.
		stream_socket_shutdown($IN_SOCK, STREAM_SHUT_WR);
	}
	if (!$noWaitResponse) {
		expect('/IN.*connection closed/');
	} else {
		usleep(500000);
	}
	if (!$noWaitResponse) {
		recv_in();
	}
}

function recv_in($numLines = null)
{
	global $IN_SOCK, $DEBUG;
	if (!$numLines) {
		$numLines = 1e10;
	}
	$lines = array();
	for ($i = 0; $i < $numLines && !feof($IN_SOCK); $i++) {
		$lines[] = fgets($IN_SOCK, 1024);
	}
	$ret = join("", $lines);
	$ret = trim($ret);
	if (!$DEBUG) {
		$ret = preg_replace('/\(0x\w+\)/', '(0x*)', $ret);
		$ret = preg_replace('/(online |offline |FAKE |=> |\[)[\d.]+:/s', '$1*:', $ret);
	}
	echo preg_replace('/^/m', 'IN ==> ', $ret) . "\n";
	if (feof($IN_SOCK)) {
		fclose($IN_SOCK);
		$IN_SOCK = null;
	}
}

function disconnect_in()
{
	global $IN_SOCK;
	echo "IN :: Mandatory disconnecting.\n";
	fclose($IN_SOCK);
	expect('/IN.*closed/');
}

function send_wait($data, $nowait = false)
{
	global $WAIT_SOCK;
	$data = trim(preg_replace('/^[ \t]+/m', '', $data));
	echo preg_replace('/^/m', 'WA <-- ', $data) . "\n";
	$WAIT_SOCK = fsockopen("127.0.0.1", 8088);
	fwrite($WAIT_SOCK, "$data\n");
	fflush($WAIT_SOCK);
	if (!$nowait) expect('/WAIT.*registered|WAIT.*marker received/');
}

function recv_wait()
{
	global $WAIT_SOCK;
	$ret = trim(stream_get_contents($WAIT_SOCK));
	$ret = preg_replace('/^((Last-Modified|Expires): )[^\r\n]+/m', '$1***', $ret);
	$ret = preg_replace('/(: )"(\d\d\d\d\d+\.\d\d+)"/s', '${1}<cursor>', $ret);
	echo preg_replace('/^/m', 'WA --> ', $ret) . "\n";
	disconnect_wait(true);
}

function disconnect_wait($noWait = false)
{
	global $WAIT_SOCK;
	echo "WA :: Disconnecting.\n";
	fclose($WAIT_SOCK);
	if (!$noWait) {
		expect('/WAIT.*closed/');
	}
}
