<?php
// Check if the request method is POST
$isPost = $_SERVER['REQUEST_METHOD'] === 'POST';
$submittedText = '';

if ($isPost && isset($_POST['text'])) {
    $submittedText = htmlspecialchars($_POST['text'], ENT_QUOTES, 'UTF-8');
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title><?php echo $isPost ? 'Text Display - OuiTransfer' : 'Welcome - OuiTransfer'; ?></title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:ital,opsz,wght@0,14..32,100..900;1,14..32,100..900&display=swap');

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Inter', sans-serif;
            background: linear-gradient(135deg, #0c0c0c 0%, #1a1a2e 50%, #16213e 100%);
            color: #e0e0e0;
            line-height: 1.6;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            position: relative;
            overflow-x: hidden;
        }

        /* Animated background particles */
        body::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: 
                radial-gradient(2px 2px at 20px 30px, rgba(119, 141, 169, 0.3), transparent),
                radial-gradient(2px 2px at 40px 70px, rgba(119, 141, 169, 0.2), transparent),
                radial-gradient(1px 1px at 90px 40px, rgba(119, 141, 169, 0.4), transparent),
                radial-gradient(1px 1px at 130px 80px, rgba(119, 141, 169, 0.3), transparent);
            background-repeat: repeat;
            background-size: 150px 100px;
            animation: particles 20s linear infinite;
            z-index: 1;
        }

        @keyframes particles {
            0% { transform: translate(0, 0); }
            100% { transform: translate(-150px, -100px); }
        }

        .container {
            max-width: 800px;
            width: 90%;
            padding: 3rem;
            background: rgba(26, 26, 30, 0.9);
            backdrop-filter: blur(20px);
            border-radius: 24px;
            border: 1px solid rgba(119, 141, 169, 0.2);
            box-shadow: 
                0 25px 50px -12px rgba(0, 0, 0, 0.5),
                0 0 0 1px rgba(255, 255, 255, 0.05);
            position: relative;
            z-index: 2;
            animation: fadeInUp 0.8s ease-out;
        }

        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .header {
            text-align: center;
            margin-bottom: 2.5rem;
        }

        .logo {
            font-size: 2.5rem;
            font-weight: 700;
            background: linear-gradient(135deg, #778da9 0%, #a8c4e0 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            margin-bottom: 0.5rem;
        }

        .subtitle {
            color: #9ca3af;
            font-size: 1.1rem;
            font-weight: 400;
        }

        .form-container {
            background: rgba(255, 255, 255, 0.03);
            padding: 2rem;
            border-radius: 16px;
            border: 1px solid rgba(119, 141, 169, 0.1);
            margin-bottom: 2rem;
        }

        .form-group {
            margin-bottom: 1.5rem;
        }

        label {
            display: block;
            color: #778da9;
            font-weight: 500;
            margin-bottom: 0.5rem;
            font-size: 0.95rem;
        }

        input[type="text"], textarea {
            width: 100%;
            padding: 1rem 1.25rem;
            background: rgba(0, 0, 0, 0.3);
            border: 2px solid rgba(119, 141, 169, 0.2);
            border-radius: 12px;
            color: #e0e0e0;
            font-size: 1rem;
            transition: all 0.3s ease;
            font-family: 'Inter', sans-serif;
        }

        input[type="text"]:focus, textarea:focus {
            outline: none;
            border-color: #778da9;
            box-shadow: 0 0 0 4px rgba(119, 141, 169, 0.1);
            transform: translateY(-2px);
        }

        .submit-btn {
            background: linear-gradient(135deg, #778da9 0%, #5a7aa0 100%);
            color: white;
            padding: 1rem 2rem;
            border: none;
            border-radius: 12px;
            font-weight: 600;
            font-size: 1rem;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(119, 141, 169, 0.3);
            width: 100%;
        }

        .submit-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(119, 141, 169, 0.4);
        }

        .result-container {
            background: rgba(34, 197, 94, 0.1);
            border: 1px solid rgba(34, 197, 94, 0.3);
            padding: 2rem;
            border-radius: 16px;
            margin-bottom: 2rem;
        }

        .result-title {
            color: #22c55e;
            font-weight: 600;
            margin-bottom: 1rem;
            font-size: 1.2rem;
        }

        .result-text {
            background: rgba(0, 0, 0, 0.3);
            padding: 1.5rem;
            border-radius: 12px;
            color: #e0e0e0;
            word-wrap: break-word;
            border-left: 4px solid #22c55e;
            font-family: 'Inter', monospace;
            line-height: 1.7;
        }

        .info-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 1.5rem;
            margin-top: 2rem;
        }

        .info-card {
            background: rgba(255, 255, 255, 0.03);
            padding: 1.5rem;
            border-radius: 16px;
            border: 1px solid rgba(119, 141, 169, 0.1);
            transition: all 0.3s ease;
        }

        .info-card:hover {
            transform: translateY(-4px);
            box-shadow: 0 10px 30px rgba(119, 141, 169, 0.2);
            border-color: rgba(119, 141, 169, 0.3);
        }

        .info-icon {
            font-size: 2rem;
            margin-bottom: 1rem;
            display: block;
        }

        .info-title {
            color: #778da9;
            font-weight: 600;
            margin-bottom: 0.5rem;
            font-size: 1.1rem;
        }

        .info-description {
            color: #9ca3af;
            font-size: 0.95rem;
            line-height: 1.5;
        }

        .back-btn {
            background: rgba(119, 141, 169, 0.1);
            color: #778da9;
            padding: 0.75rem 1.5rem;
            border: 1px solid rgba(119, 141, 169, 0.3);
            border-radius: 8px;
            text-decoration: none;
            font-weight: 500;
            display: inline-block;
            transition: all 0.3s ease;
            margin-top: 1rem;
        }

        .back-btn:hover {
            background: rgba(119, 141, 169, 0.2);
            transform: translateX(-4px);
        }

        @media (max-width: 768px) {
            .container {
                margin: 1rem;
                padding: 2rem;
            }
            
            .logo {
                font-size: 2rem;
            }
            
            .info-container {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1 class="logo">OuiTransfer</h1>
            <p class="subtitle">Modern Text Display System</p>
        </div>

        <?php if ($isPost): ?>
            <!-- POST Request - Display submitted text -->
            <div class="result-container">
                <h2 class="result-title">✅ Text Successfully Received</h2>
                <div class="result-text">
                    <?php 
                    if (!empty($submittedText)) {
                        echo nl2br($submittedText);
                    } else {
                        echo "<em style='color: #9ca3af;'>No text was submitted</em>";
                    }
                    ?>
                </div>
            
            </div>

        <?php else: ?>
            <!-- Non-POST Request - Display form and info -->
            <div class="form-container">
                <form method="POST" action="<?php echo $_SERVER['PHP_SELF']; ?>">
                    <div class="form-group">
                        <label for="text">Enter your text:</label>
                        <input type="text" name="text" id="text" placeholder="Type your message here..." required>
                    </div>
                    <button type="submit" class="submit-btn">Submit Text</button>
                </form>
            </div>

            <div class="info-container">
                <div class="info-card">
                    <div class="info-icon">🚀</div>
                    <h3 class="info-title">Fast & Secure</h3>
                    <p class="info-description">Lightning-fast text processing with secure handling of your data using modern PHP techniques.</p>
                </div>

                <div class="info-card">
                    <div class="info-icon">🎨</div>
                    <h3 class="info-title">Modern Design</h3>
                    <p class="info-description">Beautiful, responsive interface with smooth animations and contemporary styling.</p>
                </div>

                <div class="info-card">
                    <div class="info-icon">⚡</div>
                    <h3 class="info-title">Real-time Display</h3>
                    <p class="info-description">Instantly view your submitted text with proper formatting and sanitization.</p>
                </div>

                <div class="info-card">
                    <div class="info-icon">🛡️</div>
                    <h3 class="info-title">XSS Protection</h3>
                    <p class="info-description">Built-in security measures to protect against cross-site scripting attacks.</p>
                </div>
            </div>
        <?php endif; ?>
    </div>
</body>
</html>