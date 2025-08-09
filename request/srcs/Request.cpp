# include <ctime>
# include <cstdio>
# include <climits>
# include <cstddef>
# include <cstdlib>
# include <sstream>
# include "../incs/Request.hpp"
# include "../../conf/Http.hpp"
# include "../../Connection.hpp"

Request::Request()
	:	_fd(-1), _rl(""), _rh(""), _rb(),
		_state(BEGIN), _statusCode(START), _requestDone(false)
{
}

Request::Request(int fd)
	:	_fd(fd), _rl(""), _rh(""), _rb(), _state(BEGIN), _statusCode(START),
		_requestDone(false)
{
}

bool	Request::_processType(std::string type)
{
	if (type == "")
		return true;

	if (type == "multipart/form-data")
		return setState(false, UNSUPPORTED_MEDIA_TYPE);
	if (type == "")
		return setState(false, UNSUPPORTED_MEDIA_TYPE);
	_rb.setContentType(type);
	return true;
}

bool	Request::_processBodyHeaders(Connection* conn)
{
	std::string contentType = _rh.getHeaderValue("content-type");
	if (!_processType(contentType))
		return false;

	std::string contentLengthStr = _rh.getHeaderValue("content-length");
	std::string transferEncoding = _rh.getHeaderValue("transfer-encoding");

	if (transferEncoding.empty() && contentLengthStr.empty())
		return setState(false, LENGTH_REQUIRED);

	if (!transferEncoding.empty())
		_rb.setChunked(_isChunkedTransferEncoding(transferEncoding));
	else if (!contentLengthStr.empty() && !_processContentLength())
		return false;

	if (getStatusCode() == OK)
		return true;
	if (!_connectionChecks(conn))
		return false;

	return true;
}

bool	Request::_processContentLength()
{
	std::string	contentLengthStr = _rh.getHeaderValue("content-length");

	char* end;
	unsigned long long contentLength = strtoull(contentLengthStr.c_str(), &end, 10);

	if (*end != '\0' || contentLength < 0)
		return setState(false, BAD_REQUEST);

	if (contentLength == 0)
		return setState(true, OK);

	_rb.setContentLength(contentLength);
	return true;
}

std::string stripQuotes(const char* str)
{
	if (!str)
		return "";

	std::string s(str);

	if (s.size() >= 2 && ((s[0] == '"' && s[s.size() - 1] == '"') || (s[0] == '\'' && s[s.size() - 1] == '\'')))
		s = s.substr(1, s.size() - 2);

	return s;
}

void	Request::treatUploadLocation(Connection* conn)
{
	std::string rawPath = stripQuotes(conn->uploadLocation.c_str());

	std::string tmp;
	bool slash = false;
	for (size_t i = 0; i < rawPath.size(); ++i)
	{
		if (rawPath[i] == '/')
		{
			if (!slash)
			{
				tmp += '/';
				slash = true;
			}
		}
		else
		{
			slash = false;
			tmp += rawPath[i];
		}
	}
	rawPath = tmp;

	std::string segment;
	bool skipping = true;
	std::vector<std::string> parts;
	std::istringstream iss(rawPath);

	while (std::getline(iss, segment, '/'))
	{
		if (segment.empty()) continue;
		if (skipping && segment[0] == '.')
			continue;
		skipping = false;
		parts.push_back(segment);
	}

	std::string cleaned;
	for (size_t i = 0; i < parts.size(); ++i)
		cleaned += "/" + parts[i];

	if (cleaned.empty())
		cleaned = "/";

	std::string fullPath = "/tmp" + cleaned;
	conn->uploadLocation = fullPath;

	std::string path;
	for (size_t i = 0; i < parts.size(); ++i)
	{
		path += "/" + parts[i];
		std::string current = "/tmp" + path;
		// << "Checking directory: " << current << std::endl;
		if (access(current.c_str(), F_OK) == -1)
		{
			if (mkdir(current.c_str(), 0755) == -1)
			{
				perror("mkdir");
				conn->req->setState(false, INTERNAL_SERVER_ERROR);
				return;
			}
		}
	}
}


bool	Request::_connectionChecks(Connection* conn)
{
	std::vector<std::string> allowed = conn->_getAllowedMethods();
	if (!conn->_isAllowedMethod("POST", allowed))
		return conn->req->setState(false, METHOD_NOT_ALLOWED);

	conn->getUpload();
	if (conn->uploadAuthorized)
	{
		treatUploadLocation(conn);
		_rb.create(POST_BODY, conn->uploadLocation);
		_state = BODY;
	}
	else
		return setState(false, FORBIDDEN);
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

bool	Request::headerSection(Connection* conn, Http* http)
{
	size_t end_header = _buffer.find(CRLFCRLF);
	if (end_header == std::string::npos)
		return false;

	const std::string headersStr = _buffer.substr(0, end_header + 2);
	if (headersStr.empty())
		return setState(false, BAD_REQUEST);

	_rh = RequestHeaders(headersStr);
	if (!_rh.parse())
		return setState(false, _rh.getStatusCode());

	_buffer.erase(0, end_header + 4);

	if (!conn->conServer)
		conn->findServer(http);

	if (_rl.getMethod() == "GET" || _rl.getMethod() == "DELETE")
		return setState(true, OK);

	/*
		TODO
	*/

	if (conn->checkMaxBodySize() == false)
		return setState(false, PAYLOAD_TOO_LARGE);

	if (!_processBodyHeaders(conn))
		return false;

	return true;
}

bool	Request::bodySection()
{
	// << "Processing body section...\n";
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

bool	Request::appendToBuffer(Connection* conn, Http* http, const char* data, size_t len)
{
	_buffer.append(data, len);

	bool progress = true;
	while (progress && !isRequestDone())
	{
		if (progress)
			conn->lastActivityTime = time(NULL);
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
				if (headerSection(conn, http))
					progress = true;
				break;

			case BODY:
				// << BG_YELLOW << "ON BODY SECTION" << RESET << std::endl;
				if (bodySection())
					progress = true;
				break;

			default:
				break;
		}
	}

	return true;
}