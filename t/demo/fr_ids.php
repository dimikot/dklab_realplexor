<?require_once "_common.php"?>
<html>
<head>

<style>
body { font-size: 120%; }
.board { border: 2px solid red; margin-right: 1em; width: 20%; float: left; }
.board_head { background: blue; color: white; font-weight: bold; padding: 0.2em; }
.board_body { padding: 0.2em; min-height: 2em; }
.online { border: 2px solid green; margin-right: 1em; width: 10%; float: left; }
.online_head { background: #00FF00; color: black; padding: 0.2em; }
.online_body { padding: 0.2em; min-height: 2em; }
.title { font-weight: bold; color: yellow; }
.listen { border: 2px solid blue; padding: 0.5em; width: 15%; float: left; }
.progress { float: right; }
.close { cursor: hand; cursor: pointer; }
</style>

<script type="text/javascript" src="http://rpl.<?=$_SERVER['HTTP_HOST']?>/?identifier=SCRIPT&<?=0*time()?>"></script>
<script type="text/javascript" src="static/jquery.min.js"></script>
<script>
var realplexor = new Dklab_Realplexor(
	"http://rpl.<?=$_SERVER['HTTP_HOST']?>/?<?=0*time()?>",  // URL of engine
	"demo_" // namespace
);
$(document).ready(function() {
	// Template from which we create board blocks.
	var board = $("#board");

	// Create new channel block.	
	function addListen(id) {
		var n = board.clone(true);
		board.before(n);
		$('.title', n).text(id);
		n.show();
		var num = 0;
		n[0].identifier = id;
		n[0].callback = function(result, id, cursor) {
			var b = $('.board_body', n);
			var line = document.createElement("div");
			line.innerHTML = (++num) + ": " + 
				result.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
			b.prepend(line);
		}
		realplexor.subscribe(id, n[0].callback);
		realplexor.execute();
	}
	
	// Refresh "online" block.
	function setOnline(data) {
		$("#online .online_body").html(data.join("<br/>"));
	}
	
	// Handle "close button".
	$('.close').live('click', function() {
		var n = $(this).parents('.board');
		realplexor.unsubscribe(n[0].identifier, n[0].callback);
		realplexor.execute();
		n.remove();
	});
	
	// Handle "new channel" button.
	$('.listen .button').click(function() {
		addListen($(this).parents().find('input').val());
	});

	// Subscribe to online changes.
	realplexor.setCursor("who_is_online", 0);
	realplexor.subscribe("who_is_online", function(data) {
		setOnline(data);
	});
	setOnline(<?=json_encode($mpl->cmdOnline())?>);
	
	
	// Create initial boards set.
	<?$ids = strlen(@$_GET['ids'])? explode(",", $_GET['ids']) : array("alpha", "beta")?>
	<?foreach ($ids as $id) {?>
		addListen(<?=json_encode($id)?>);
	<?}?>
	
});
</script>
</head>

<body>
<h1>Browser #1: many users are waiting for messages</h1>

<div class="online" id="online">
	<div class="online_head">
		Online channels
	</div>
	<div class="online_body">
	</div>
</div>

<div class="board" id="board" style="display:none">
	<div class="board_head">
		<img class="close" src="static/close.gif" style="float:right" />
		Channel <span class="title"></span>
		<img src="static/indicator.gif" style="position:absolute; margin:4px 0 0 4px" />
	</div>
	<div class="board_body">
	</div>
</div>

<div class="listen">
	Listen to a new ID:<br/>
	<input type="text" size="20" value="gamma"/></br>
	<input type="button" class="button" value="Start listening" />
</div>

</body>
</html>
