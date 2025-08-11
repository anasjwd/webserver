#!/usr/bin/env python3

print("Content-Type: text/html\r\n")
print()

html = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OuiTransfer</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:ital,opsz,wght@0,14..32,100..900;1,14..32,100..900&display=swap');

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        .container {
            max-width: 1200px;
            margin: 2rem auto;
            padding: 0 2rem;
        }
        body {
            background-color: #1a1a1a;;
            color: #e0e0e0;
            line-height: 1.6;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }
        h1 {
            text-align: center;
            color: #778da9;
        }
        div {
            font-size: 2rem;
            font-weight: semibold;
            color: #e0e0e0;
            text-align: center;
        }
        canvas {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: 9999;
        }
        .button{
            margin-top: 2rem;
            border: none;
            padding: 1rem 3rem;
            background-color: #333;
            border-radius: 15px;
            font-weight: semibold;
            color: #778da9;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Upload a File</h1>
        <form action="./form.html" method="post" enctype="multipart/form-data">
            <label for="file">Select a file:</label>
             <input type="file" name="file" id="file">
            
            <br><br>
            <input type="submit" value="Upload">
        </form>
    </div>
</body>
</html>
"""

print(html)
