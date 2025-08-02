#!/usr/bin/env python3
import os
import sys
import urllib.parse

print("Content-Type: text/html")
print()
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Simple Calculator</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; margin: 20px; }")
print(".result { background-color: #e8f5e8; padding: 10px; border-radius: 5px; margin: 10px 0; }")
print("input[type=text] { padding: 5px; margin: 5px; }")
print("input[type=submit] { padding: 5px 10px; background-color: #4CAF50; color: white; border: none; border-radius: 3px; cursor: pointer; }")
print("</style>")
print("</head>")
print("<body>")
print("<h1>Simple Calculator</h1>")

while(True):
    print("hello")
# Get query string
query_string = os.environ.get('QUERY_STRING', '')

if query_string:
    try:
        # Parse the query string
        params = urllib.parse.parse_qs(query_string)
        expression = params.get('expr', [''])[0]
        
        # URL decode the expression and replace spaces with +
        if expression:
            expression = urllib.parse.unquote(expression)
            expression = expression.replace(' ', '+')  # Replace spaces with +
            
            # Simple evaluation (be careful with eval in production!)
            result = eval(expression)
            print(f"<div class='result'>")
            print(f"<strong>Expression:</strong> {expression}<br>")
            print(f"<strong>Result:</strong> {result}")
            print("</div>")
    except Exception as e:
        print(f"<div class='result' style='background-color: #ffe8e8;'>")
        print(f"<strong>Error:</strong> {str(e)}")
        print("</div>")

print("<form method='GET'>")
print("<label for='expr'>Enter expression (e.g., 2+3*4):</label><br>")
print("<input type='text' id='expr' name='expr' value='' placeholder='2+3*4'>")
print("<input type='submit' value='Calculate'>")
print("</form>")

print("<br>")
print("<h3>Examples:</h3>")
print("<ul>")
print("<li><a href='?expr=2+3'>2 + 3</a></li>")
print("<li><a href='?expr=10*5'>10 * 5</a></li>")
print("<li><a href='?expr=100/4'>100 / 4</a></li>")
print("<li><a href='?expr=2**8'>2^8 (2**8)</a></li>")
print("</ul>")

print("<br>")
print("<a href='/'>Back to Home</a>")
print("</body>")
print("</html>") 