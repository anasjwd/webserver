# include <cctype>
# include <sstream>
# include <cstdlib>
# include "../incs/RequestLine.hpp"

RequestLine::RequestLine(const std::string& raw)
	:	_raw(raw), _statusCode(START)
{
}

bool	RequestLine::_validateMethod()
{
	const std::string allowedMethods[] = {"GET", "POST", "DELETE"};
	const size_t count = sizeof(allowedMethods)/sizeof(allowedMethods[0]);

	for (size_t i = 0; i < count; ++i)
		if (_method == allowedMethods[i])
			return true;

	return setState(false, METHOD_NOT_ALLOWED);
}

static bool	isValidUriChar(char c)
{
	if (isalnum(c))
		return true;

	const char allowed[] = {'-', '.', '_', '~', ':', '/', '?', '#', '[', ']',
		'@', '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '=', '%'};

	for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); ++i)
		if (c == allowed[i])
			return true;

	return false;
}

bool	RequestLine::_validateUri()
{
	if (_uri.empty())
		return setState(false, BAD_REQUEST);

	if (_uri.length() > MAX_URI_LENGTH)
		return setState(false, URI_TOO_LONG);

	if (_uri[0] != '/')
		return setState(false, BAD_REQUEST);

	for (size_t i = 0; i < _uri.size(); ++i)
	{
		if (!isValidUriChar(_uri[i]))
			return setState(false, BAD_REQUEST);

		if (_uri[i] == '%')
		{
			if (i + 2 >= _uri.size() || !isxdigit(_uri[i +1 ])
				|| !isxdigit(_uri[i + 2]))
				return setState(false, BAD_REQUEST);
			i += 2;
		}
	}

	return true;
}

bool	RequestLine::_validateVersion()
{
	if (_version == HTTP_VERSION)
		return setState(true, OK);

	if (_version.substr(0, 5) != "HTTP/")
		return setState(false, BAD_REQUEST);

	return setState(false, VERSION_NOT_SUPPORTED);
}

void	RequestLine::clear()
{
	_raw.clear();
	_uri.clear();
	_method.clear();
	_version.clear();
	_statusCode = START;
	_queryParams.clear();
}

bool	RequestLine::parse()
{
	if (_raw.length() < 14)
		return setState(false, BAD_REQUEST);

	size_t firstSpace = _raw.find(' ');
	if (firstSpace == std::string::npos)
		return setState(false, BAD_REQUEST);

	size_t secondSpace = _raw.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos || secondSpace == firstSpace + 1)
		return setState(false, BAD_REQUEST);

	_method = _raw.substr(0, firstSpace);
	_uri = _raw.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	_version = _raw.substr(secondSpace + 1);

	
	if (!_validateMethod() || !_validateUri() || !_validateVersion())
		return false;

	size_t queryPos = _uri.find('?');
	if (queryPos != std::string::npos)
	{
		_parseQueryString();
		_uri = _raw.substr(firstSpace + 1, queryPos);
	}

	return setState(true, OK);
}

std::string	urlDecode(const std::string& str)
{
	std::string	result;

	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '%' && i + 2 < str.size())
		{
			if (!isxdigit(str[i+1]) || !isxdigit(str[i+2]))
				return "";
			char hex[3] = {str[i+1], str[i+2], '\0'};
			char *end;
			unsigned long val = strtoul(hex, &end, 16);
			if (*end != '\0')
				return "";
			result += static_cast<char>(val);
			i += 2;
		} 
		else if (str[i] == '+')
			result += ' ';
		else
			result += str[i];
	}
	return result;
}

void	RequestLine::_parseQueryString()
{
	std::string			key;
	std::string			pair;
	std::string			value;

	size_t				queryPos = _uri.find('?');
	std::string			queryStr = _uri.substr(queryPos + 1);
	std::istringstream	iss(queryStr);

	while (std::getline(iss, pair, '&'))
	{
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos)
		{
			key = pair.substr(0, equalPos);
			value = pair.substr(equalPos + 1);
			_queryParams[urlDecode(key)] = urlDecode(value);
		}
		else if (!pair.empty())
			_queryParams[urlDecode(pair)] = "";
	}
}

const std::string&	RequestLine::getMethod() const
{
	return _method;
}

const std::string&	RequestLine::getUri() const
{
	return _uri;
}

const std::string&	RequestLine::getVersion() const
{
	return _version;
}

HttpStatusCode		RequestLine::getStatusCode() const
{
	return _statusCode;
}

bool	RequestLine::setState(bool tof, HttpStatusCode code)
{
	_statusCode = code;
	return tof;
}

const std::map<std::string, std::string>&	RequestLine::getQueryParams() const
{
	return _queryParams;
}
