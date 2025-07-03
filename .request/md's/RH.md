REQUEST HEADER GRAMMAR:

Standard from RFC 7230 Section 3.2
=========================================================
Header-Field = Field-Name ":" OWS Field-Value OWS CRLF
=========================================================

Field-Name

	Case-insensitive token identifying the header (e.g., Host, Content-Type, User-Agent).
	Must not contain whitespace.
	No spaces or tabs allowed between the field-name and the colon (:).
	Servers must reject requests with whitespace between field-name and colon with a 400 Bad Request response.

Colon (:)

	Separates the field-name and field-value.
	Must immediately follow the field-name without any spaces in between.

OWS (Optional Whitespace)

	Zero or more spaces or horizontal tabs allowed before and after the field-value.
	Leading and trailing whitespace around the field-value should be ignored when parsing.

Field-Value

	Can be any sequence of visible ASCII characters, spaces, and tabs, excluding CR and LF.
	Historically, field-values could be folded across multiple lines by starting continuation lines with whitespace (obs-fold), but this is deprecated and should be rejected or replaced with spaces.
	Parsers should trim leading and trailing OWS from the field-value.

CRLF

	Each header field line ends with carriage return + line feed (\r\n).
	The end of the header section is indicated by an empty line (a CRLF on a line by itself).
