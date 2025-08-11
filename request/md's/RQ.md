METHOD SP URI SP HTTP_VERSION

LONGEST METHOD DELETE		=> 6
SP							=> 1
MAX URI SIZE				=> 2048
HTTP_VERSION "HTTP/1.1"		=> 8
CRLF						=> 2

* If the length of the buffer is longer than 2066 (6 + 1 + 2048 + 1 + 8 + 2), and still didn't find \r\n, MAX_URI, directly. It should be fine also using the REQUEST_TIMEOUT.