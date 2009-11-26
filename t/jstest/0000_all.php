<table width="100%" height="100%" cellpadding="0" cellspacing="0" border="0">
<tr height="1%">
    <td>
        <?include "contrib/init.php"?>
	<form enctype="multipart/form-data" id="form">
	    <table border=0 cellpadding="0" cellspacing="0">
	    <tr id="runAllButton"><td colspan="2">
	        <input type="button" onclick="runAllTests()" value="Run all the tests" style="width:100%">
	    </td></tr>
	    </table>
	</form>
        <script type="text/javascript" language="JavaScript" src="JsTest/JsTestIterator.js"></script>
        <script>
        // Run this function on timeout or on button press - for old Opera 
        // (need time to initialize IFRAME).
        JsTest.analyzed = true; // workaround for init.php
        function runAllTests() {
            <?
            $urls = array();
            foreach (glob('*.jst') as $v) {
                $urls[] = preg_replace('{//+}s', '/', dirname($_SERVER['SCRIPT_NAME']) . "/$v" . (isset($_GET['debug'])? "?debug" : ''));
            }
            ?>
            JsTestIterator.run(
                window.frames.result,
                <?=json_encode($urls)?>,
                function(text) {
                	var form = document.getElementById('result_form');
                	form.result.value = text;
                	form.submit();
                }
            );
        }
        if ((""+document.location).match(/immediate/)) {
            setTimeout("runAllTests()", 500);
        }
        </script>
        
        <input type="checkbox" onclick="JsTestIterator.stopOnError = this.checked">Stop on the first error<br>
    </td>
</tr>
<tr>
    <td>        
        <iframe src="contrib/empty.txt" name="result" style="width:100%; height:100%"></iframe>
        <form action="contrib/tests_result.php" id="result_form" target="result" method="post" style="display:none">
        <input type="hidden" name="result" value="">
        </form>
    </td>
</tr>
</table>