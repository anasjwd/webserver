#!/usr/bin/env php
<?php
echo "Content-Type: text/html\n\n";

if (isset($_GET['num'])) {
    echo "<h1>hello world</h1>";
    setcookie('num', $_GET['num'], time() + 3600);
    header('Location: ' . strtok($_SERVER["REQUEST_URI"], '?'));
    exit;
}

$num = isset($_COOKIE['num']) ? $_COOKIE['num'] : 0;

echo "<!DOCTYPE html>\n";
echo "<html lang=\"en\">\n";
echo "<head>\n";
echo "    <meta charset=\"UTF-8\">\n";
echo "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
echo "    <title>PHP Cookie Test</title>\n";
echo "</head>\n";
echo "<body>\n";
echo "    <h1>PHP Cookie Test</h1>\n";
echo "    <p>This page stores the input number in a cookie and always loads the input value from the cookie.</p>\n";
echo "    <form action=\"\" method=\"get\">\n";
echo "        <label for=\"num\">Enter a number:</label>\n";
echo "        <input type=\"number\" name=\"num\" id=\"num\" value=\"" . htmlspecialchars($num) . "\">\n";
echo "        <input type=\"submit\" value=\"Save\">\n";
echo "    </form>\n";
echo "    <h2>Cookie Info</h2>\n";
echo "    <ul>\n";
if (!empty($_COOKIE)) {
    foreach ($_COOKIE as $key => $value) {
        echo "        <li><strong>" . htmlspecialchars($key) . ":</strong> " . htmlspecialchars($value) . "</li>\n";
    }
} else {
    echo "        <li>No cookies set.</li>\n";
}
echo "    </ul>\n";
echo "    <h2>Environment Info</h2>\n";
echo "    <ul>\n";
foreach ($_ENV as $key => $value) {
    echo "        <li><strong>$key:</strong> $value</li>\n";
}
echo "    </ul>\n";
echo "    <hr>\n";
echo "    <p>Current time: " . date('Y-m-d H:i:s') . "</p>\n";
echo "    <p>PHP version: " . phpversion() . "</p>\n";
echo "</body>\n";
echo "</html>\n";
?>