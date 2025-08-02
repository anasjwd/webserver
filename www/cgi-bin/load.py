import os
import cgitb

cgitb.enable()

print("Content-Type: text/html; charset=utf-8\r\n\r\n")
print()

uploaded_file_path = os.environ.get('UPLOADED_FILE_PATH', '')

if uploaded_file_path and os.path.exists(uploaded_file_path):
    filename = os.path.basename(uploaded_file_path)
    value = f"'{filename}' a été uploadé avec succès et enregistré à '{uploaded_file_path}'"
else:
    value = "Aucun fichier n'a été téléchargé ou le fichier n'existe pas."

html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Upload</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@100..900&display=swap');

        body {{
            font-family: 'Inter', sans-serif;
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            color: #333;
        }}
        .container {{
            text-align: center;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h2> {value} </h2>
        <a href="/">Back home</a>
    </div>
</body>
</html>
"""

print(html_content)
