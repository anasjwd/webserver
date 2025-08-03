<?php
if (!isset($_COOKIE['numOfClicks'])) {
    setcookie('numOfClicks', '0', time() + 31556952, '/');
    $numClicks = 0;
}
else {
    $numClicks = $_COOKIE['numOfClicks'];
}
?>

<!DOCTYPE html>
<html>

<head>
  <title>Click And Test Cookie</title>
</head>

<body>
  <button id="clicker">Click Me</button>
  <p id="counter">Clicks: <?php echo $numClicks; ?></p>

	<script>
		let clicks = <?php echo (int)$numClicks; ?>;
		document.getElementById("clicker").onclick = function() {
			clicks++;
			document.getElementById("counter").textContent = "Clicks: " + clicks;
			let date = new Date();
			date.setTime(date.getTime() + (365*24*60*60*1000));
			let expires = "expires=" + date.toUTCString();
			document.cookie = "numOfClicks=" + clicks + ";" + expires + "; path=/";
		};
	</script>

</body>

</html>
