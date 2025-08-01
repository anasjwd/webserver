#!/usr/bin/php
<?php

// Start with default counter value
$counter = 1;

// Get the HTTP method
$method = getenv('REQUEST_METHOD');

// Read cookies from the environment
$cookieHeader = getenv('HTTP_COOKIE');
if ($cookieHeader) {
    $cookies = explode(';', $cookieHeader);
    foreach ($cookies as $cookie) {
        list($name, $value) = array_map('trim', explode('=', $cookie, 2));
        if ($name === 'counter') {
            $counter = (int)$value;
            break;
        }
    }
}

// Only increment the counter if the method is POST (i.e., user clicked the button)
if ($method === 'POST') {
    $counter++;
}

$message = ($counter === 1) ? "Welcome! This is your first visit." : "You have visited this page $counter times.";

// Send headers
echo "Content-Type: text/html\r\n";
echo "Set-Cookie: counter=$counter\r\n";
echo "\r\n";
?>

<head>
    <meta charset="UTF-8">
    <title>Visit Counter</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            text-align: center;
            background-color: #fff;
            padding: 2rem;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        .button {
            margin-top: 1rem;
            padding: 0.5rem 1rem;
            background-color: #007bff;
            color: #fff;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
        .button:hover {
            background-color: #0056b3;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Visit Counter</h2>
        <p><?php echo $message; ?></p>
        <form action="" method="post">
            <button type="submit" name="reset" class="button">Reset Visit Count</button>
        </form>
    </div>
</body>
</html>
