#!/usr/bin/env python3
import os
import sys
import cgi

print("Content-Type: text/html")
print()

print("""
<html>
<head>
    <title>File Upload</title>
</head>
<body>
    <h1>File Upload Handler</h1>
""")

if os.environ.get('REQUEST_METHOD') == 'POST':
    try:
        form = cgi.FieldStorage()
        if 'file' in form:
            fileitem = form['file']
            if fileitem.filename:
                # Save the uploaded file
                filename = os.path.basename(fileitem.filename)
                with open(f"/tmp/{filename}", "wb") as f:
                    f.write(fileitem.file.read())
                print(f"<p>File '{filename}' uploaded successfully!</p>")
            else:
                print("<p>No file selected</p>")
        else:
            print("<p>No file field found</p>")
    except Exception as e:
        print(f"<p>Error: {e}</p>")
else:
    print("""
    <form method="POST" enctype="multipart/form-data">
        <input type="file" name="file">
        <input type="submit" value="Upload">
    </form>
    """)

print("""
</body>
</html>
""")
