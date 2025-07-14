#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print()

print("""
<html>
<head>
    <title>Environment Variables</title>
</head>
<body>
    <h1>CGI Environment Variables</h1>
    <table border="1">
        <tr><th>Variable</th><th>Value</th></tr>
""")

# Print all environment variables
for key, value in os.environ.items():
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("""
    </table>
</body>
</html>
""")
