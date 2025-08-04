#!/bin/bash

echo "Content-Type: text/html"
echo "Status: 200 OK"
echo ""
echo "<html>"
echo "<head><title>Shell CGI Test</title></head>"
echo "<body>"
echo "<h1>Shell CGI Test</h1>"
echo "<p>This is a test CGI script written in Bash.</p>"
echo "<h2>Environment Variables:</h2>"
echo "<ul>"
for var in REQUEST_METHOD QUERY_STRING CONTENT_TYPE CONTENT_LENGTH SERVER_NAME SERVER_PORT; do
    if [ -n "${!var}" ]; then
        echo "<li><strong>$var:</strong> ${!var}</li>"
    fi
done
echo "</ul>"
echo "<h2>Request Method:</h2>"
echo "<p>Method: $REQUEST_METHOD</p>"
if [ "$REQUEST_METHOD" = "POST" ]; then
    echo "<h2>POST Data:</h2>"
    echo "<pre>"
    cat
    echo "</pre>"
fi
echo "</body>"
echo "</html>" 