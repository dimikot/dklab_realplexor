// Constructor. 
// Create new Dklab_Realplexor object.
function Dklab_Realplexor(fullUrl, namespace, viaDocumentWrite)
{
	// Current JS library version.
	var VERSION = "1.23";

	// Detect current page hostname.
	var host = document.location.host;
	
	// Assign initial properties.
	if (!this.constructor._registry) this.constructor._registry = {}; // all objects registry
	this.version = VERSION;
	this._map = {};
	this._realplexor = null;
	this._namespace = namespace;
	this._login = null;
	this._iframeId = "mpl" + (new Date().getTime());
	this._iframeTag = 
		'<iframe'
		+ ' id="' + this._iframeId + '"'
		+ ' onload="' + 'Dklab_Realplexor' + '._iframeLoaded(&quot;' + this._iframeId + '&quot;)"'
		+ ' src="' + fullUrl + '?identifier=IFRAME&amp;HOST=' + host + '&amp;version=' + this.version + '"'
		+ ' style="position:absolute; width:200px; height: 200px; left: -1000px"' +
		'></iframe>';
	this._iframeCreated = false;
	this._needExecute = false;
	this._executeTimer = null;
	
	// Register this object in the registry (for IFRAME onload callback).
	this.constructor._registry[this._iframeId] = this;
	
	// Validate realplexor URL.
	if (!fullUrl.match(/^\w+:\/\/([^/]+)/)) {
		throw 'Dklab_Realplexor constructor argument must be fully-qualified URL, ' + fullUrl + ' given.';
	}
	var mHost = RegExp.$1;
	if (mHost != host && mHost.lastIndexOf("." + host) != mHost.length - host.length - 1) {
		throw 'Due to the standard XMLHttpRequest security policy, hostname in URL passed to Dklab_Realplexor (' + mHost + ') must be equals to the current host (' + host + ') or be its direct sub-domain.';
	} 
	
	// Create IFRAME if requested.
	if (viaDocumentWrite) {
		document.write(this._iframeTag);
		this._iframeCreated = true;
	}
	
	// Allow realplexor's IFRAME to access outer window.
	document.domain = host;	
}

// Static function. 
// Called when a realplexor iframe is loaded.
Dklab_Realplexor._iframeLoaded = function(id)
{
	var th = this._registry[id];
	// use setTimeout to let IFRAME JavaScript code some time to execute.
	setTimeout(function() {
		var iframe = document.getElementById(id);
		th._realplexor = iframe.contentWindow.Dklab_Realplexor_Loader;
		if (th.needExecute) {
			th.execute();
		}
	}, 50);
}

// Set active login.
Dklab_Realplexor.prototype.logon = function(login) {
	this._login = login;
}

// Set the position from which we need to listen a specified ID.
Dklab_Realplexor.prototype.setCursor = function(id, cursor) {
	if (!this._map[id]) this._map[id] = { cursor: null, callbacks: [] };
	this._map[id].cursor = cursor;
	return this;
}

// Subscribe a new callback to specified ID.
// To apply changes and reconnect to the server, call execute()
// after a sequence of subscribe() calls.
Dklab_Realplexor.prototype.subscribe = function(id, callback) {
	if (!this._map[id]) this._map[id] = { cursor: null, callbacks: [] };
	var chain = this._map[id].callbacks;
	for (var i = 0; i < chain.length; i++) {
		if (chain[i] === callback) return;
	}
	chain.push(callback);
	return this;
}

// Unsubscribe a callback from the specified ID.
// You do not need to reconnect to the server (see execute()) 
// to stop calling of this callback.
Dklab_Realplexor.prototype.unsubscribe = function(id, callback) {
	if (!this._map[id]) return;
	if (callback == null) {
		this._map[id].callbacks = [];
		return;
	}
	var chain = this._map[id].callbacks;
	for (var i = 0; i < chain.length; i++) {
		if (chain[i] === callback) {
			chain.splice(i, 1);
			return;
		}
	}
	return this;
}

// Reconnect to the server and listen for all specified IDs.
// You should call this method after a number of calls to subscribe().
Dklab_Realplexor.prototype.execute = function() {
	// Control IFRAME creation.
	if (!this._iframeCreated) {
		var div = document.createElement('DIV');
		div.innerHTML = this._iframeTag;
		document.body.appendChild(div);
		this._iframeCreated = true;
	}
	
	// Check if the realplexor is ready (if not, schedule later execution).
	if (this._executeTimer) {
		clearTimeout(this._executeTimer);
		this._executeTimer = null;
	}
	var th = this;
	if (!this._realplexor) {
		this._executeTimer = setTimeout(function() { th.execute() }, 30);
		return;
	}
	
	// Realplexor loader is ready, run it.
	this._realplexor.execute(
		this._map, 
		this.constructor._callAndReturnException, 
		(this._login != null? this._login + "_" : "") + (this._namespace != null? this._namespace : "")
	);
}

// This is a work-around for stupid IE. Unfortunately IE cannot
// catch exceptions which are thrown from the different frame
// (in most cases). Please see
// http://blogs.msdn.com/jaiprakash/archive/2007/01/22/jscript-exceptions-not-handled-thrown-across-frames-if-thrown-from-a-expando-method.aspx
Dklab_Realplexor._callAndReturnException = function(func, args) {
	try {
		func.apply(null, args);
		return null;
	} catch (e) {
		return "" + e;
	}
}
