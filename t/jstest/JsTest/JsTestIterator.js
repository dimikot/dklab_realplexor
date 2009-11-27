/**
 * JsTest: JavaScript asynchronous testing framework (tests iteration).
 *
 * @license LGPL
 * @author Dmitry Koterov, http://en.dklab.ru
 */

var JsTestIterator = {
    iframe: null,        // IFRAME to use for testing
    tests: [],           // tests to perform
    reports: [],         // tests reports ('success', 'error', 'skip')
    curTest: -1,         // current test number
    nextDt: 100,         // delay to the next iteration
    stopOnError: false,  // if true, do not run the next test on error
    onResult: null,      // callback: called when all tests are finished (with text message)
    addUrl: '',          // additional string added to each of the URLs
    
    // Called from child JsTest.
    report: function(result) {
        var bgr = {
            'ERROR': '#FFBBBB',
            'SKIP': '#FFFF00',
            'SUCCESS': '#EEFFEE'
        };
        if (this.curTest >= 0) {
            this.reports[this.curTest] = result;
        }
        if (this.stopOnError && result == "error") {
            // Skip next tests on error.
            return;
        }
        // Run the next test.        
        var th = this;
        var closure = function() { 
            th.curTest++;
            var name = th.tests[th.curTest];
            if (!name) {
                var text = '';
                text += '<b>Browser: ' + window.navigator.userAgent + "</b><br><br>\n\n";
                for (var i = 0; i < th.tests.length; i++) {
                    var r = th.reports[i].toUpperCase();
                    var url = th.tests[i];
                    text += '<div style="background: ' + bgr[r] + '">Testing <a target="_blank" href="' + url + '">' + url.replace(/.*\/|\?.*/, '') + "</a>... " + r + '</div>\n';
                }
                text += "\n<br>All tests are finished!\n";
                var d = th.iframe.document;
                d.write(text);
                d.close();
                if (th.onResult) th.onResult(text);
            } else {
                // Run the next test.
                th.iframe.window.location = (name 
                    + (name.indexOf('?') >= 0? '&' : '?') + new Date().getTime()
                    + (th.addUrl? '&' + th.addUrl : '')
                );
            }
        };
        setTimeout(closure, th.nextDt);
    },
    
    // Runs a test sequence.
    run: function(iframe, tests, onResult) {
        this.iframe = iframe;
        this.tests = tests;
        this.reports = [];
        this.curTest = -1;
        this.onResult = onResult;
        this.report();
    }
}
