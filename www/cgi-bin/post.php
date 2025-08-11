<?php
// Check if the request method is POST
$isPost = $_SERVER['REQUEST_METHOD'] === 'POST';

// echo $isPost;
$submittedText = '';

if ($isPost && isset($_POST['text'])) {
    $submittedText = htmlspecialchars($_POST['text'], ENT_QUOTES, 'UTF-8');
}
if ($isPost)
    echo $submittedText;
else
    echo "not post method";


?>
<br>  
<a href='/cgi-bin'>Back to Home</a>

