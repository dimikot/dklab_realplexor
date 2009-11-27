/**
 * JsTest: JavaScript asynchronous testing framework (individual test).
 *
 * @license LGPL
 * @author Dmitry Koterov, http://en.dklab.ru
 */

var JsTest = {
    analyzed: false,
    maxTimeout: 6000,
    maxTimeoutHandler: null,
    
    getDiv: function(name) {
        var div = document.getElementById(name);
        if (!div) throw new Error('No element with id=' + name + '!');
        return div;
    },
    
    report: function(result) {
        this.analyzed = true;
        var testDiv = this.getDiv('TEST');
        testDiv.innerHTML += " - <b>" + result.toUpperCase() + "</b>";
        testDiv.className = result;
        if (parent.JsTestIterator) {
            parent.JsTestIterator.report(result);
        }
    },
    
    write: function(text) {
        var fileDiv = this.getDiv('FILE');
        if (!fileDiv.isResult) {
            var div = document.createElement('PRE');
            div.isResult = true;
            div.id = "FILE";
            fileDiv.parentNode.replaceChild(div, fileDiv);
            fileDiv = div;
        }

        text = "" + text;
        if (!fileDiv.innerHTML.length) {
            // If the text contains leading empty lines and div is not filled yet, skip them.
            text = text.replace(new RegExp('^[ \t\r\n]*[\r\n]+', 'g'), '');
        }

        // \ -> <br> replacement - for Opera 7.20 bug :-(
        // &nbsp; before <br> - for IE (it eats multiple <br>'s inside <pre>).
        text = text.replace(/ /g, '&nbsp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/\r?\n/g, '&nbsp;<br>');
        fileDiv.innerHTML += text;
        
        // Reset timeout on each write.
        this.setMaxTimeout(this.maxTimeout);
    },
    
    skip: function() {
        this.report('skip');
    },
    
    analyze: function() {
        if (this.analyzed) return;
        this.analyzed = true;
        var fileDiv = this.getDiv('FILE');
        var expectDiv = this.getDiv('EXPECT');
        var a = this.removeSpaces(fileDiv.innerHTML);
        var b = this.removeSpaces(expectDiv.innerHTML);
        if (a != b) {
            fileDiv.className = "error";
            this.report('error');
        } else {
            this.report('success');
        }
    },
    
    setMaxTimeout: function(maxTimeout) {
    	var old = this.maxTimeout;
        this.maxTimeout = maxTimeout;
        this.initialize();
        return old;
    },
    
    initialize: function() {
        // Auto-analyze a result on timeout.
        this.analyzed = false;
        if (this.maxTimeoutHandler) {
            clearTimeout(this.maxTimeoutHandler);
        }
        // IE5 crashed when we use a closure in setTimeout()!!! So - use string.
        this.maxTimeoutHandler = setTimeout("JsTest.analyze()", JsTest.maxTimeout);
    },
    
    removeSpaces: function(s) {
    	// Safari bug work-arount: damned Safari does not mass-remove \x00A0 by regexps! :-(
    	var old = null;
    	while (old != s) {
    		old = s;
    		s = s.replace('\u00A0', '').replace("\xa0", "");
    	}
    	return s.replace(/\s+|&nbsp;|<br\s*\/?>/gi, '');
    }
}
