<?php
session_start();

if (!isset($_SESSION['numOfClicks'])) {
    $_SESSION['numOfClicks'] = 0;
}
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $_SESSION['numOfClicks']++;
    echo $_SESSION['numOfClicks'];
    exit;
}
?>
<!DOCTYPE html>
<html>

<head>
    <title>Click And Test Session</title>
</head>

<body>
    <button id="clicker">Click Me</button>
    <p id="counter">Session Clicks: <?php echo $_SESSION['numOfClicks']; ?></p>

    <script>
        document.getElementById("clicker").onclick = function() {
            const data = 'click=true';
            fetch(window.location.href, 
            {method: 'POST',
                body : data

            })
                .then(response => response.text())
                .then(count => {
                    document.getElementById("counter").textContent = "Session Clicks: " + count;
                });
        };
    </script>

</body>

</html>
