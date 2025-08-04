#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print("Status: 200 OK")
print("")
print("<html>")
print("<head><title>Python CGI Test</title></head>")
print("<body>")
print("<h1>Python CGI Test</h1>")
print("<p>This is a test CGI script written in Python.</p>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}:</strong> {value}</li>")
print("</ul>")
print("<h2>Request Method:</h2>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>")
if os.environ.get('REQUEST_METHOD') == 'POST':
    print("<h2>POST Data:</h2>")
    print("<pre>")
    print(sys.stdin.read())
    print("</pre>")
print("</body>")
print("</html>") 