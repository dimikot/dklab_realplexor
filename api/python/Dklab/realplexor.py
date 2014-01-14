import socket
import json
import re

class Dklab_Realplexor(object):
    """  Dklab_Realplexor python API. """

    def __init__(self, host, port, namespace=None, identifier='identifier'):
        """
        Create new realplexor API instance.

        Keyword arguments:
        host -- Host of IN line.
        port -- Port of IN line (if 443, SSL is used).
        namespace -- Namespace to use.
        identifier -- Use this "identifier" marker instead of the default one.
        """
        self._login = None
        self._password = None
        self._timeout = 5
        self._host = host
        self._port = port
        self._namespace = namespace
        self._identifier = identifier

    def logon(self, login, password):
        """
        Set login and password to access Realplexor (if the server needs it).
        This method does not check credentials correctness.
        """
        self._login = login
        self._password = password
        # All keys must always be login-prefixed!
        self._namespace = self._login + "_" + self._namespace

    def send(self, idsAndCursors, data, showOnlyForIds=None):
        """
        Send data to realplexor.
        Throw Dklab_Realplexor_Exception in case of error.

        idsAndCursors -- Target IDs in form of: dictionary(id1 => cursor1, id2 => cursor2, ...)
                                     of dictionary(id1, id2, id3, ...). If sending to a single ID,
                                     you may pass it as a plain string, not dictionary.
        data -- Data to be sent (any format, e.g. nested dictionaries are OK).
        showOnlyForIds  -- Send this message to only those who also listen any of these IDs.
                                     This parameter may be used to limit the visibility to a closed
                                     number of cliens: give each client an unique ID and enumerate
                                     client IDs in $showOnlyForIds to not to send messages to others.
        """
        data = json.dumps(data)
        pairs = []
        for id in idsAndCursors:
            if type(id) == type(1):
                id = cursor # this is NOT cursor, but ID!
                cursor = None
            if re.search('^\w+$', id) is None:
                raise Dklab_Realplexor_Exception("Identifier must be alphanumeric, \"%s\" given" % id)
            try:
                cursor = idsAndCursors[id]
            except:
                cursor = None
            id = (self._namespace or '') + id
            if cursor is not None:
                try:
                    i = float(cursor)
                except ValueError:
                    raise Dklab_Realplexor_Exception("Cursor must be numeric, \"%s\" given" % cursor)
                pairs.append("%s:%s" % (cursor,id))
            else:
                pairs.append(id)
        if isinstance(showOnlyForIds, (list, tuple)):
            for id in showOnlyForIds:
                pairs.append("*" + (self._namespace or '') + id)
        self._send(",".join(pairs), data)

    def cmdOnlineWithCounters(self, idPrefixes=None):
        """
        Return list of online IDs (keys) and number of online browsers
        for each ID. (Now "online" means "connected just now", it is
        very approximate; more precision is in TODO.)

        idPrefixes -- If set, only online IDs with these prefixes are returned.
        """
        # Add namespace.
        idPrefixes = [] if idPrefixes is None else list(idPrefixes)
        if self._namespace:
            if not idPrefixes:
                idPrefixes = [""] # if no prefix passed, we still need namespace prefix
            idPrefixes = [self._namespace + (value or "") for value in idPrefixes]
        # Send command.
        resp = self._sendCmd("online" + (" " + " ".join(idPrefixes) if idPrefixes else ""))
        if not resp.strip():
            return {}
        # Parse the result and trim namespace.
        result = {}
        for line in resp.split("\n"):
            try:
                id, counter = tuple(line.split(" "))
            except:
                continue
            if not id:
                continue
            if self._namespace and id.startswith(self._namespace):
                id = id[len(self._namespace):]
            result[id] = counter
        return result

    def cmdOnline(self, idPrefixes=None):
        """
        Return list of online IDs.

        idPrefixes --  If set, only online IDs with these prefixes are returned.
        """
        return self.cmdOnlineWithCounters(idPrefixes).keys()

    def cmdWatch(self, fromPos, idPrefixes=None):
        """
        Return all Realplexor events (e.g. ID offline/offline changes)
        happened after fromPos cursor.

        fromPos -- Start watching from this cursor.
        idPrefixes -- Watch only changes of IDs with these prefixes.
        Returns list of dict("event": ..., "pos": ..., "id": ...).
        """
        idPrefixes = [] if idPrefixes is None else list(idPrefixes)
        if not fromPos:
            fromPos = 0
        if not re.match('^[\d.]+$', fromPos.__str__()):
            raise Dklab_Realplexor_Exception("Position value must be numeric, \"%s\" given" % fromPos)
        # Add namespaces.
        if self._namespace:
            if not idPrefixes:
                idPrefixes = [""] # if no prefix passed, we still need namespace prefix
            idPrefixes = [self._namespace + (value or "") for value in idPrefixes]
        # Execute.
        resp = self._sendCmd(("watch %s " % fromPos) + (" ".join(idPrefixes) if idPrefixes else ""))
        if not resp.strip():
            return []
        # Parse.
        events = []
        for line in resp.strip().split("\n"):
            m = re.match('^(\w+)\s+([^:]+):(\S+)\s*$', line)
            if not m:
                # Cannot parse the event
                continue
            event, pos, id = m.group(1), m.group(2), m.group(3)
            # Cut off namespace.
            if fromPos and self._namespace and id.startswith(self._namespace):
                id = id[len(self._namespace):]
            events.append(dict(event=event, pos=pos, id=id))
        return events

    def _sendCmd(self, cmd):
        """
        Internal method.
        Send IN command.

        cmd -- Command to send.
        Returns server IN response.
        """
        return self._send(None, cmd + "\n")

    def _send(self, identifier, body):
        """
        Internal method.
        Send specified data to IN channel. Return response data.
        Throw Dklab_Realplexor_Exception in case of error.

        Keyword arguments:
        identifier -- If set, pass this identifier string.
        data -- Data to be sent.

         Returns response from IN line.
         """
        # Build HTTP request.
        headers = "X-Realplexor: %s=" % self._identifier
        if self._login:
            header += self._login + ':' + self._password + '@'
        headers += (identifier or '') + "\r\n"

        data = "POST / HTTP/1.1\r\nHost: %s\r\nContent-Length: %i\r\n%s\r\n%s\r\n" % (
                        self._host, len(body), headers, body)
        # Proceed with sending.
        result = None
        host = "ssl://" + self._host if self._port == 443 else  self._host
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(self._timeout)
        try:
            s.connect((host, int(self._port)))
            s.sendall(data)
            s.shutdown(socket.SHUT_WR)
            result = ''
            while True:
                tmp = s.recv(4096)
                if not tmp:
                    break
                result += tmp
            s.close()
        except Exception as e:
            raise Dklab_Realplexor_Exception(e)
        # Analyze the result.
        if result:
            regexp = re.compile("\r?\n\r?\n", re.S)
            try:
                headers, body = tuple(re.split(regexp, result, 2))
            except:
                raise Dklab_Realplexor_Exception("Non-HTTP response received:\n" + result)
            m = re.match('^HTTP/[\d.]+\s+((\d+)[^\r\n]*)', headers)
            if not m:
                raise Dklab_Realplexor_Exception("Non-HTTP response received:\n" + result)
            if m.group(2) != "200":
                raise Dklab_Realplexor_Exception("Request failed: " + m.group(1) + "\n" + body);
            m = re.search('Content-Length:\s*(\d+)', headers)
            if not m:
                raise Dklab_Realplexor_Exception("No Content-Length header in response headers:\n" + headers)
            needLen = int(m.group(1))
            recvLen = len(body)
            if recvLen != needLen:
                raise Dklab_Realplexor_Exception("Response length (%s) is different than specified in Content-Length header (%s): possibly broken response\n" % (recvLen, needLen))
            return body
        return result

class Dklab_Realplexor_Exception(Exception):
    """ Realplexor-dedicated exception class. """
    pass