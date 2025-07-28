#!/usr/bin/env python3
import os
import sys
import cgi
import cgitb

# Enable error reporting
cgitb.enable()

print("Content-Type: text/html")
print()
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Upload Test</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; margin: 20px; }")
print(".info { background-color: #e8f4f8; padding: 10px; border-radius: 5px; margin: 10px 0; }")
print("input[type=file] { margin: 10px 0; }")
print("input[type=submit] { padding: 5px 10px; background-color: #4CAF50; color: white; border: none; border-radius: 3px; cursor: pointer; }")
print("</style>")
print("</head>")
print("<body>")
print("<h1>Upload Test CGI</h1>")

# Check if this is a POST request
if os.environ.get('REQUEST_METHOD') == 'POST':
    print("<div class='info'>")
    print("<h2>POST Request Received</h2>")
    
    # Get content length
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    print(f"<p><strong>Content Length:</strong> {content_length} bytes</p>")
    
    # Get content type
    content_type = os.environ.get('CONTENT_TYPE', '')
    print(f"<p><strong>Content Type:</strong> {content_type}</p>")
    
    # Read POST data
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<p><strong>POST Data:</strong></p>")
        print(f"<pre>{post_data[:500]}{'...' if len(post_data) > 500 else ''}</pre>")
    
    print("</div>")
else:
    print("<div class='info'>")
    print("<h2>GET Request</h2>")
    print("<p>This page accepts POST requests for testing file uploads.</p>")
    print("</div>")

print("<form method='POST' enctype='multipart/form-data'>")
print("<label for='file'>Select a file:</label><br>")
print("<input type='file' id='file' name='file'><br>")
print("<input type='submit' value='Upload File'>")
print("</form>")

print("<br>")
print("<h3>Environment Variables:</h3>")
print("<ul>")
for key, value in sorted(os.environ.items()):
    if key.startswith('HTTP_') or key in ['REQUEST_METHOD', 'CONTENT_TYPE', 'CONTENT_LENGTH', 'QUERY_STRING']:
        print(f"<li><strong>{key}:</strong> {value}</li>")
print("</ul>")

print("<br>")
print("<a href='/'>Back to Home</a>")
print("</body>")
print("</html>") 