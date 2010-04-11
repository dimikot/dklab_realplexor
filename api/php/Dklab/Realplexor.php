<?php
/**
 * Dklab_Realplexor PHP API.
 *
 * @version 1.31
 */
class Dklab_Realplexor
{
	private $_timeout = 5;
	private $_host;
	private $_port;
	private $_identifier;
	private $_login;
	private $_password;

	/**
	 * Create new realplexor API instance.
	 *
	 * @param string $host        Host of IN line.
	 * @param string $port        Port of IN line (if 443, SSL is used).
	 * @param strinf $namespace   Namespace to use. 
	 * @param string $identifier  Use this "identifier" marker instead of the default one.
	 */
	public function __construct($host, $port, $namespace = null, $identifier = "identifier")
	{
		$this->_host = $host;
		$this->_port = $port;
		$this->_namespace = $namespace;
		$this->_identifier = $identifier;
		if (version_compare(PHP_VERSION, "5.2.1", "<")) {
			throw new Dklab_Realplexor_Exception("You should use PHP 5.2.1 and higher to run this library");
		}
	}
	
	/**
	 * Set login and password to access Realplexor (if the server needs it).
	 * This method does not check credentials correctness.
	 *
	 * @param string $login
	 * @param string $password
	 * @return void
	 */
	public function logon($login, $password)
	{
		$this->_login = $login;
		$this->_password = $password;
		// All keys must always be login-prefixed!
		$this->_namespace = $this->_login . "_" . $this->_namespace;
	}

	/**
	 * Send data to realplexor.
	 * Throw Dklab_Realplexor_Exception in case of error.
	 *
	 * @param mixed $idsAndCursors    Target IDs in form of: array(id1 => cursor1, id2 => cursor2, ...)
	 *                               of array(id1, id2, id3, ...). If sending to a single ID,
	 *                               you may pass it as a plain string, not array.
	 * @param mixed $data            Data to be sent (any format, e.g. nested arrays are OK).
	 * @param array $showOnlyForIds  Send this message to only those who also listen any of these IDs.
	 *                               This parameter may be used to limit the visibility to a closed
	 *                               number of cliens: give each client an unique ID and enumerate
	 *                               client IDs in $showOnlyForIds to not to send messages to others.
	 * @return void
	 */                              
	public function send($idsAndCursors, $data, $showOnlyForIds = null)
	{
		$data = json_encode($data);
		$pairs = array();
		foreach ((array)$idsAndCursors as $id => $cursor) {
			if (is_int($id)) {
				$id = $cursor; // this is NOT cursor, but ID!
				$cursor = null;
			}
			if (!preg_match('/^\w+$/', $id)) {
				throw new Dklab_Realplexor_Exception("Identifier must be alphanumeric, \"$id\" given");
			}
			$id = $this->_namespace . $id;
			if ($cursor !== null) {
				if (!is_numeric($cursor)) {
					throw new Dklab_Realplexor_Exception("Cursor must be numeric, \"$cursor\" given");
				}
				$pairs[] = "$cursor:$id";
			} else {
				$pairs[] = $id;
			}
		}
		if (is_array($showOnlyForIds)) {
			foreach ($showOnlyForIds as $id) {
				$pairs[] = "*" . $this->_namespace . $id;
			}
		}
		$this->_send(join(",", $pairs), $data);
	}
	
	/**
	 * Return list of online IDs (keys) and number of online browsers
	 * for each ID. (Now "online" means "connected just now", it is
	 * very approximate; more precision is in TODO.)
	 *
	 * @param array $idPrefixes   If set, only online IDs with these prefixes are returned.
	 * @return array              List of matched online IDs (keys) and online counters (values).
	 */
	public function cmdOnlineWithCounters($idPrefixes = null)
	{
		// Add namespace.
		$idPrefixes = $idPrefixes !== null? (array)$idPrefixes : array();
		if (strlen($this->_namespace)) {
			if (!$idPrefixes) $idPrefixes = array(""); // if no prefix passed, we still need namespace prefix
			foreach ($idPrefixes as $i => $idp) {
				$idPrefixes[$i] = $this->_namespace . $idp;			
			}
		}
		// Send command.
		$resp = $this->_sendCmd("online" . ($idPrefixes? " " . join(" ", $idPrefixes) : ""));
		if (!strlen(trim($resp))) return array();
		// Parse the result and trim namespace.
		$result = array();
		foreach (explode("\n", $resp) as $line) {
			@list ($id, $counter) = explode(" ", $line);
			if (!strlen($id)) continue;
			if (strlen($this->_namespace) && strpos($id, $this->_namespace) === 0) {
				$id = substr($id, strlen($this->_namespace));
			}
			$result[$id] = $counter;
		}
		return $result;
	}

	/**
	 * Return list of online IDs.
	 *
	 * @param array $idPrefixes   If set, only online IDs with these prefixes are returned.
	 * @return array              List of matched online IDs.
	 */
	public function cmdOnline($idPrefixes = null)
	{
		return array_keys($this->cmdOnlineWithCounters($idPrefixes));
	}
	
	/**
	 * Return all Realplexor events (e.g. ID offline/offline changes)
	 * happened after $fromPos cursor.
	 *
	 * @param string $fromPos        Start watching from this cursor.
	 * @param array $idPrefixes        Watch only changes of IDs with these prefixes.
	 * @return array                   List of array("event" => ..., "cursor" => ..., "id" => ...).
	 */
	public function cmdWatch($fromPos, $idPrefixes = null)
	{
		$idPrefixes = $idPrefixes !== null? (array)$idPrefixes : array();
		if (!$fromPos) {
			$fromPos = 0;
		}
		if (!preg_match('/^[\d.]+$/', $fromPos)) {
			throw new Dklab_Realplexor_Exception("Position value must be numeric, \"$fromPos\" given");
		}
		// Add namespaces.
		if (strlen($this->_namespace)) {
			if (!$idPrefixes) $idPrefixes = array(""); // if no prefix passed, we still need namespace prefix
			foreach ($idPrefixes as $i => $idp) {
				$idPrefixes[$i] = $this->_namespace . $idp;			
			}
		}
		// Execute.
		$resp = $this->_sendCmd("watch $fromPos" . ($idPrefixes? " " . join(" ", $idPrefixes) : ""));
		if (!trim($resp)) return array();
		$resp = explode("\n", trim($resp));
		// Parse.
		$events = array();
		foreach ($resp as $line) {
			if (!preg_match('/^ (\w+) \s+ ([^:]+):(\S+) \s* $/sx', $line, $m)) {
				trigger_error("Cannot parse the event: \"$line\"");
				continue;
			}
			list ($event, $pos, $id) = array($m[1], $m[2], $m[3]);
			// Cut off namespace.
			if ($fromPos && strlen($this->_namespace) && strpos($id, $this->_namespace) === 0) {
				$id = substr($id, strlen($this->_namespace));
			}
			$events[] = array(
				'event' => $event,
				'pos'   => $pos,
				'id'    => $id,
			);
		}
		return $events;
	}
	
	/**
	 * Internal method.
	 * Send IN command.
	 *
	 * @param string $cmd   Command to send.
	 * @return string       Server IN response.
	 */
	private function _sendCmd($cmd)
	{
		return $this->_send(null, "$cmd\n");
	}

	/**
	 * Internal method.
	 * Send specified data to IN channel. Return response data.
	 * Throw Dklab_Realplexor_Exception in case of error.
	 *
	 * @param string $identifier  If set, pass this identifier string.
	 * @param string $data        Data to be sent.
	 * @return string             Response from IN line.
	 */
	private function _send($identifier, $body)
	{
		// Build HTTP request.
		$headers = "X-Realplexor: {$this->_identifier}=" 
			. ($this->_login? $this->_login . ":" . $this->_password . '@' : '')
			. ($identifier? $identifier : "")
			. "\r\n";
		$data = ""
			. "POST / HTTP/1.1\r\n"
			. "Host: " . $this->_host . "\r\n"
			. "Content-Length: " . $this->_strlen($body) . "\r\n"
			. $headers
			. "\r\n"
			. $body;
		// Proceed with sending.
		$old = ini_get('track_errors');
		ini_set('track_errors', 1);
		$result = null;
		try {
			$host = $this->_port == 443? "ssl://" . $this->_host : $this->_host;
			$f = @fsockopen($host, $this->_port, $errno, $errstr, $this->_timeout);
			if (!$f) {
				throw new Dklab_Realplexor_Exception("Error #$errno: $errstr");
			}
			if (@fwrite($f, $data) === false) {
				throw new Dklab_Realplexor_Exception($php_errormsg);
			}
			if (!@stream_socket_shutdown($f, STREAM_SHUT_WR)) {
				throw new Dklab_Realplexor_Exception($php_errormsg);
				break;
			}
			$result = @stream_get_contents($f);
			if ($result === false) {
				throw new Dklab_Realplexor_Exception($php_errormsg);
			}
			if (!@fclose($f)) {
				throw new Dklab_Realplexor_Exception($php_errormsg);
			}
			ini_set('track_errors', $old);
		} catch (Exception $e) {
			ini_set('track_errors', $old);
			throw $e;
		}
		// Analyze the result.
		if ($result) {
			@list ($headers, $body) = preg_split('/\r?\n\r?\n/s', $result, 2);
			if (!preg_match('{^HTTP/[\d.]+ \s+ ((\d+) [^\r\n]*)}six', $headers, $m)) {
				throw new Dklab_Realplexor_Exception("Non-HTTP response received:\n" . $result);
			}
			if ($m[2] != 200) {
				throw new Dklab_Realplexor_Exception("Request failed: " . $m[1] . "\n" . $body);
			}
			if (!preg_match('/^Content-Length: \s* (\d+)/mix', $headers, $m)) {
				throw new Dklab_Realplexor_Exception("No Content-Length header in response headers:\n" . $headers);
			}
			$needLen = $m[1];
			$recvLen = $this->_strlen($body);
			if ($needLen != $recvLen) {
				throw new Dklab_Realplexor_Exception("Response length ($recvLen) is different than specified in Content-Length header ($needLen): possibly broken response\n");
			}
			return $body;
		}
		return $result;
	}
	
	/**
	 * Wrapper for mbstring-overloaded strlen().
	 *
	 * @param string $body
	 * @return int
	 */
	private function _strlen($body)
	{
		return function_exists('mb_orig_strlen')? mb_orig_strlen($body) : strlen($body);
	}
}

/**
 * Realplexor-dedicated exception class.
 */
class Dklab_Realplexor_Exception extends Exception
{
}
