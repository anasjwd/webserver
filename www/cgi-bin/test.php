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

echo "<h2>Request Method:</h2>\n";
echo "<p>REQUEST_METHOD: " . getenv('REQUEST_METHOD') . "</p>\n";

echo "<h2>POST Data:</h2>\n";
if (getenv('REQUEST_METHOD') == 'POST') {
    $content_length = intval(getenv('CONTENT_LENGTH'));
    echo "<p>Content Length: $content_length</p>\n";
    
    if ($content_length > 0) {
        $post_data = '';
        $bytes_read = 0;
        while ($bytes_read < $content_length) {
            $chunk = fread(STDIN, $content_length - $bytes_read);
            if ($chunk === false) break;
            $post_data .= $chunk;
            $bytes_read += strlen($chunk);
        }
        echo "<p>POST Data: " . htmlspecialchars($post_data) . "</p>\n";
        
        // Parse POST data
        parse_str($post_data, $post_vars);
        echo "<h3>Parsed POST Variables:</h3>\n";
        echo "<ul>\n";
        foreach ($post_vars as $key => $value) {
            echo "<li><strong>$key:</strong> $value</li>\n";
        }
        echo "</ul>\n";
    }
} else {
    echo "<p>No POST data (GET request)</p>\n";
}

echo "<h2>Environment Variables:</h2>\n";
echo "<ul>\n";
foreach ($_ENV as $key => $value) {
    echo "<li><strong>$key:</strong> $value</li>\n";
}
echo "</ul>\n";

echo "<h2>HTTP_COOKIE Environment Variable:</h2>\n";
echo "<p>HTTP_COOKIE: " . (isset($_ENV['HTTP_COOKIE']) ? $_ENV['HTTP_COOKIE'] : 'NOT SET') . "</p>\n";

echo "<h2>PHP $_COOKIE Array:</h2>\n";
echo "<ul>\n";
foreach ($_COOKIE as $key => $value) {
    echo "<li><strong>$key:</strong> $value</li>\n";
}
echo "</ul>\n";

echo "<h2>All Environment Variables (getenv):</h2>\n";
echo "<ul>\n";
$env_vars = getenv();
foreach ($env_vars as $key => $value) {
    echo "<li><strong>$key:</strong> $value</li>\n";
}
echo "</ul>\n";

echo "<br>\n";
echo "<a href='/'>Back to Home</a>\n";
echo "</body>\n";
echo "</html>\n";
?> 