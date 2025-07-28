#!/usr/bin/env php
<?php
echo "Content-Type: text/html\n\n";
echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "<title>PHP CGI Test</title>\n";
echo "</head>\n";
echo "<body>\n";
echo "<h1>PHP CGI Test</h1>\n";
echo "<p>This is a PHP CGI script running on our webserver!</p>\n";
echo "<p>Current time: " . date('Y-m-d H:i:s') . "</p>\n";
echo "<p>PHP version: " . phpversion() . "</p>\n";
echo "<h2>Environment Variables:</h2>\n";
echo "<ul>\n";
foreach ($_ENV as $key => $value) {
    echo "<li><strong>$key:</strong> $value</li>\n";
}
echo "</ul>\n";
echo "<br>\n";
echo "<a href='/'>Back to Home</a>\n";
echo "</body>\n";
echo "</html>\n";
?> 