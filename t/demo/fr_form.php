<html>
<head>
<style>
.form { float: left; margin-right: 3em; }
</style>
<script type="text/javascript" src="static/jquery.min.js"></script>
<script>
$(".send").live("click", function() {
	var ids = $.map($('.identifier', this.form), function(e) { return e.value? e.value : null });
	var message = this.form.message.value;
	var repeat = this.form.repeat.value;
	var loader = $('img', this.form);
	loader.show();
	$.ajax({
		type: "POST",
		url: "ajax_post.php", 
		data: { ids: ids.join(","), message: message, repeat: repeat }, 
		complete: function(xhr, status) { 
			loader.hide();
			if (xhr.responseText) alert("Error: " + xhr.responseText); 
		}
	});
});
</script>
</head>

<body>
<h1>Browser #2: user adds a message</h1>

<p style="clear:both; color: red">
IMPORTANT: There are no direct interactions between these frames.
Think these frames are different browsers of different users.
When you press a button, your message is sent via server-side dklab_realplexor engine.
</p>


<?$ids = strlen(@$_GET['ids'])? explode(",", $_GET['ids']) : array("alpha", "beta")?>
<?foreach (array_merge($ids, array("gamma,{$ids[0]}")) as $id) {?>
<form onsubmit="return false" class="form">
<table>
<tr>
	<td>Channel to send to:</td>
	<td>
		<input type="text" class="identifier" value="<?=$id?>" />
	</td>
</tr>
<tr>
	<td>Message:</td>
	<td><input type="text" name="message" value="Hello." /></td>
</tr>
<tr>
	<td colspan="2">
		<input type="button" value="Send message" class="send" />
		<select name="repeat">
		<?for ($i = 1; $i <= 5; $i++) {?>
			<option value="<?=$i?>"><?=$i?></option>
		<?}?>
		</select>
		times
		<img src="static/loading_green.gif" style="display:none">
	</td>
</tr>
</table>
</form>
<?}?>


</body>
</html>