# include <ctime>
# include <cstdio>
# include <cstddef>
# include <cstdlib>
# include <sstream>
# include "../incs/Request.hpp"
# include "../../conf/Http.hpp"
# include "../../Connection.hpp"
# include "../../response/include/ResponseHandler.hpp"

Request::Request()
	:	_fd(-1), _rl(""), _rh(""), _rb(),
		_state(BEGIN), _statusCode(START), _requestDone(false)
{
}

Request::Request(int fd)
	:	_fd(fd), _rl(""), _rh(""), _rb(),
		_state(BEGIN), _statusCode(START), _requestDone(false)
{
}

bool	Request::_processBodyHeaders()
{
	std::string contentType = _rh.getHeaderValue("content-type");
	if (!contentType.empty())
		_rb.setContentType(contentType);

	std::string contentLengthStr = _rh.getHeaderValue("content-length");
	std::string transferEncoding = _rh.getHeaderValue("transfer-encoding");

	if (!transferEncoding.empty())
		_rb.setChunked(_isChunkedTransferEncoding(transferEncoding));
	else if (!contentLengthStr.empty())
		return _processContentLength();
	else
		return setState(false, LENGTH_REQUIRED);

	return true;
}

bool	Request::_processContentLength()
{
	std::string	contentLengthStr = _rh.getHeaderValue("content-length");

	char* end;
	unsigned long long contentLength = strtoull(contentLengthStr.c_str(), &end, 10);

	if (*end != '\0')
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


bool	Request::_validateMethodBodyCompatibility(Connection* conn)
{
	const std::string& method = _rl.getMethod();
	bool hasBody = _rb.getContentLength() > 0 || _rb.isChunked();
	bool hasContentLength = !_rh.getHeaderValue("content-length").empty();

	if (!_connectionChecks(conn))
		return false;

	if (hasBody)
		_rb.setExpected();
	else if (method == "POST" && hasContentLength)
		return setState(true, OK);

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

	std::string file;
	std::string filePath;
	struct stat	fileStat;
	const Location* location = conn->getLocation();
	std::string root = ResponseHandler::_getRootPath(conn);
	if (_rl.getMethod() == "DELETE")
	{
		file = ResponseHandler::_buildFilePath(_rl.getUri() , root, location);
		conn->getUpload();
		Request::treatUploadLocation(conn);
		filePath = conn->uploadLocation;
		filePath = filePath + "/" + file.substr((file.rfind("/") + 1));
	}
	else
		filePath = ResponseHandler::_buildFilePath(_rl.getUri(), root, location);
	if (stat(filePath.c_str(), &fileStat) == -1)
		return setState(false, NOT_FOUND);

	std::vector<std::string> allowed = conn->_getAllowedMethods();
	if (!conn->_isAllowedMethod(_rl.getMethod(), allowed))
		return conn->req->setState(false, METHOD_NOT_ALLOWED);

	if (_rl.getMethod() == "GET" || _rl.getMethod() == "DELETE")
		return setState(true, OK);
	if (conn->checkMaxBodySize() == false)
		return setState(false, PAYLOAD_TOO_LARGE);
	if (!_processBodyHeaders() || !_validateMethodBodyCompatibility(conn))
		return false;

	return true;
}

bool	Request::bodySection()
{
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
				if (bodySection())
					progress = true;
				break;

			default:
				break;
		}
	}

	return true;
}
