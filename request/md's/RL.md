REQUEST LINE GRAMMAR:

Standard from RFC 7230 Section 3.1.1
=========================================================
Request-Line = Method SP Request-URI SP HTTP-Version CRLF
=========================================================

Method

	Case-sensitive token (usually uppercase, e.g., GET, POST, DELETE).
	Server should check if the method is allowed (e.g., from config).
	Common supported methods: GET, POST, DELETE.

SP (Space)

	Exactly one ASCII space character (0x20) between tokens.
	No tabs or multiple spaces allowed.

Request-URI

	Case-sensitive (path component).
	Usually starts with a slash '/' for origin-form requests.
	Should enforce a maximum length (e.g., 2048 characters).
	Percent-encoding may be present and should be handled appropriately.

HTTP-Version

	Case-sensitive string starting with "HTTP/".
	Only "HTTP/1.1" supported by your server.
	Reject unsupported versions with appropriate error.

CRLF

	Request line must end with carriage return + line feed (\r\n).


NOTES:

	* Prevent path traversal in URI.