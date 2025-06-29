#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print()

print("""
<html>
<head>
    <title>Calculator CGI</title>
</head>
<body>
    <h1>Calculator CGI Script</h1>
    <p>This is a simple calculator CGI script.</p>
""")

# Get query string
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print(f"<p>Query string: {query_string}</p>")

# Get POST data if any
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<p>POST data: {post_data}</p>")

print("""
    <form method="GET">
        <input type="number" name="a" placeholder="First number">
        <select name="op">
            <option value="+">+</option>
            <option value="-">-</option>
            <option value="*">*</option>
            <option value="/">/</option>
        </select>
        <input type="number" name="b" placeholder="Second number">
        <input type="submit" value="Calculate">
    </form>
""")

# Process calculation
if query_string:
    try:
        params = dict(item.split('=') for item in query_string.split('&'))
        a = float(params.get('a', 0))
        b = float(params.get('b', 0))
        op = params.get('op', '+')
        
        result = 0
        if op == '+':
            result = a + b
        elif op == '-':
            result = a - b
        elif op == '*':
            result = a * b
        elif op == '/' and b != 0:
            result = a / b
        else:
            result = "Error: Division by zero"
        
        print(f"<p>Result: {a} {op} {b} = {result}</p>")
    except:
        print("<p>Error processing calculation</p>")

print("""
</body>
</html>
""")
