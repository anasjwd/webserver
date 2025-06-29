# include <cerrno>
# include <climits>
# include <cstddef>
# include <cstdlib>
#include <ctime>
# include "../incs/Request.hpp"

Request::Request()
	:	_fd(-1), _rl(""), _rh(""), _rb(), _state(BEGIN), _statusCode(START),
		_buffer(), _requestDone(false), _lastActivityTime(time(NULL))
{
}

Request::Request(int fd)
	:	_fd(fd), _rl(""), _rh(""), _rb(), _state(BEGIN), _statusCode(START),
		_buffer(), _requestDone(false), _lastActivityTime(time(NULL))
{
}

bool	Request::isMultipart(const std::string& contentType)
{
	if (contentType == "")
		return false;
	size_t semi_colon = contentType.find(";");
	if (semi_colon != std::string::npos && contentType.substr(0, semi_colon) == "multipart/form-data")
		return true;
	return false;
}

bool	Request::_processBodyHeaders()
{
	std::string contentType = _rh.getHeaderValue("content-type");
	if (!contentType.empty())
		_rb.setContentType(contentType);
	if (isMultipart(contentType))
	{
		_rb.setMultipart(true);
		_rb.extractBoundary(contentType);
	}

	std::string contentLengthStr = _rh.getHeaderValue("content-length");
	std::string transferEncoding = _rh.getHeaderValue("transfer-encoding");

	if (!transferEncoding.empty())
	{
		_rb.setChunked(_isChunkedTransferEncoding(transferEncoding));
		if (_rb.isChunked())
		{
			if (!_processChunkedTransfer())
				return false;
			return true;
		}
		return setState(false, BAD_REQUEST);
	}
	else if (!contentLengthStr.empty())
		return _processContentLength();
	else
	{
		// TODO: Should be developped to fix all cases.
		std::string method = _rl.getMethod();
		if (method == "POST")
			return setState(false, LENGTH_REQUIRED);
		else if (method == "GET" || method == "DELETE")
			return setState(true, OK);
	}
		
	return true;
}

bool	Request::_processContentLength()
{
	std::string	contentLengthStr = _rh.getHeaderValue("content-length");
	if (contentLengthStr.empty())
		return true;

	char* end;
	unsigned long long contentLength = strtoull(contentLengthStr.c_str(), &end, 10);

	if (*end != '\0' || contentLengthStr.empty())
		return setState(false, BAD_REQUEST);

	if (contentLength > MAX_BODY_SIZE)
		return setState(false, PAYLOAD_TOO_LARGE);

	if (contentLength == 0 && (_rl.getMethod() == "GET" || _rl.getMethod() == "DELETE"))
		return setState(true, OK);

	_rb.setContentLength(contentLength);
	return true;
}

bool	Request::_processChunkedTransfer()
{
	std::string	transferEncoding = _rh.getHeaderValue("transfer-encoding");
	if (transferEncoding.empty())
		return false;

	_rb.setChunked(_isChunkedTransferEncoding(transferEncoding));
	if (!_rb.isChunked())
		return false;
	return true;
}

bool	Request::_validateMethodBodyCompatibility()
{
	// TODO: Should be developped to fix all cases (Second check).
	const std::string& method = _rl.getMethod();
	bool hasBody = _rb.getContentLength() > 0 || _rb.isChunked();

	if (!hasBody && method == "POST")
		return setState(false, LENGTH_REQUIRED);

	if (!hasBody && (method == "GET" || method == "DELETE"))
		return setState(true, OK);

	if (hasBody)
		_rb.setExpected();

	return true;
}

bool	Request::_isChunkedTransferEncoding(const std::string& transferEncoding)
{
	if (transferEncoding.empty())
		return false;

	size_t chunkedPos = transferEncoding.rfind("chunked");
	if (chunkedPos == std::string::npos)
		return false;

	std::string afterChunked = transferEncoding.substr(chunkedPos + 7);
	if (afterChunked.find_first_not_of(" ,\t\r\n") != std::string::npos)
		return false;

	return true;
}

void	Request::clear()
{
	_rl.clear();
	_rh.clear();
	_rb.clear();
	_state = BEGIN;
	_buffer.clear();
	_statusCode = START;
}

bool	Request::stateChecker() const
{
	HttpStatusCode	curr = getStatusCode();
	if (curr == OK || curr == START)
		return true;

	return false;
}

bool	Request::isRequestDone() const
{
	if (_state == COMPLETE || _state == ERROR)
		return true;
	return false;
}

void	Request::setFd(int fd)
{
	_fd = fd;
}

const int&	Request::getFd() const
{
	return _fd;
}

bool	Request::checkForTimeout() const
{
	time_t currentTime = time(NULL);
	if (currentTime - _lastActivityTime > 10)
		return true;
	return false;
}

time_t	Request::getLastActivityTime() const
{
	return _lastActivityTime;
}

void	Request::setLastActivityTime(time_t time)
{
	_lastActivityTime = time;
}

const RequestState&	Request::getState() const
{
	return _state;
}

const HttpStatusCode&	Request::getStatusCode() const
{
	return _statusCode;
}

const RequestLine&	Request::getRequestLine() const
{
	return _rl;
}

const RequestBody&	Request::getRequestBody() const
{
	return _rb;
}

const RequestHeaders&	Request::getRequestHeaders() const
{
	return _rh;
}

bool	Request::setState(bool tof, HttpStatusCode code)
{
	_statusCode = code;

	if (stateChecker() == false)
		_state = ERROR;
	else if (_statusCode == OK)
		_state = COMPLETE;

	return tof;
}

bool	Request::lineSection()
{
	size_t crlf_pos = _buffer.find(CRLF);

	if (crlf_pos == std::string::npos)
		return false;

	_rl = RequestLine(_buffer.substr(0, crlf_pos));
	if (!_rl.parse())
		return setState(false, _rl.getStatusCode());

	_buffer.erase(0, crlf_pos + 2);
	_state = HEADERS;
	return true;
}

bool	Request::headerSection()
{
	size_t end_header = _buffer.find(END_HEADER);

	if (end_header == std::string::npos)
		return false;

	std::string headersStr = _buffer.substr(0, end_header + 2);
	if (headersStr.empty())
		return setState(false, BAD_REQUEST);

	_rh = RequestHeaders(headersStr);
	if (!_rh.parse())
		return setState(false, _rh.getStatusCode());

	_buffer.erase(0, end_header + 4);

	if (!_processBodyHeaders() || !_validateMethodBodyCompatibility())
		return false;

	if (_state != COMPLETE)
	{
		_state = BODY;
	}
	return true;
}

bool	Request::bodySection()
{
	if (!_rb.isExpected() && !_buffer.empty())
		return setState(false, BAD_REQUEST);

	if (!_buffer.empty())
	{
		if (!_rb.receiveData(_buffer.c_str(), _buffer.size()))
			return setState(false, _rb.getStatusCode());
		_buffer.clear();
	}

	if (_rb.isCompleted())
		return setState(true, OK);

	return false;
}

// TODO: TIMEOUT CHECK FROM CORE SERVER.

bool	Request::appendToBuffer(const char* data, size_t len)
{
	_buffer.append(data, len);

	bool progress = true;
	while (progress && !isRequestDone())
	{
		if (progress)
			setLastActivityTime(time(NULL));
		progress = false;
		switch (_state)
		{
			case BEGIN:
				if (!_buffer.empty())
				{
					_state = LINE;
					progress = true;
				}
				break;

			case LINE:
				if (lineSection())
					progress = true;
				break;

			case HEADERS:
				if (headerSection())
					progress = true;
				break;

			case BODY:
				if (bodySection())
					progress = true;
				break;

			case COMPLETE:
				return true;
			default:
				break;
		}
	}

	return true;
}
