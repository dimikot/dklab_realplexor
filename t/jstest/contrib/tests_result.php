<?php
$text = @$_POST['result'];
if (ini_get('magic_quotes_gpc')) $text = stripslashes($text);

if (!$text) die("No result specified!");
if (strlen($text) > 10000) die("Too long result!");
$result = strip_tags($text);
$result = preg_replace('/^[^\r\n]+ \s SUCCESS (\r?\n|$)/mx', '', $result);

$hash = (preg_match('/ERROR/s', $result)? "error" : "success") . '_' . md5($result);

$dir = "/tmp/jstest_result";
@mkdir($dir, 0777);

$f = fopen("$dir/$hash.txt", "w");
if (!$f) return;
fwrite($f, $result);
fclose($f);

echo $text;
echo "<br>Results are saved for later analyze.";
?>
