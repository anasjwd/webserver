#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print()
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>CGI Environment Variables</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; margin: 20px; }")
print("table { border-collapse: collapse; width: 100%; }")
print("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }")
print("th { background-color: #f2f2f2; }")
print("</style>")
print("</head>")
print("<body>")
print("<h1>CGI Environment Variables</h1>")
print("<table>")
print("<tr><th>Variable</th><th>Value</th></tr>")

# Sort environment variables for better display
env_vars = sorted(os.environ.items())
for key, value in env_vars:
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("</table>")
print("<br>")
print("<a href='/'>Back to Home</a>")
print("</body>")
print("</html>") 