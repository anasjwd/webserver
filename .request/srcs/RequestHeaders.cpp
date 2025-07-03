# include <string>
# include <cctype>
# include <sstream>
# include "../incs/RequestHeaders.hpp"

RequestHeaders::RequestHeaders(const std::string& raw)
	:	_raw(raw), _hasHost(false), _statusCode(START)
{
}

std::string	RequestHeaders::_trimWhitespace(const std::string& str)
{
	size_t	first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return "";
	size_t	last = str.find_last_not_of(" \t");
	return str.substr(first, (last - first + 1));
}

bool	RequestHeaders::_isValidHeaderName(const std::string& name)
{
	if (name.empty())
		return false;

	if (!isalpha(name[0]))
		return false;

	for (size_t i = 1; i < name.size(); ++i)
	{
		char c = name[i];
		if (!(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '!'
			|| c == '#' || c == '$' || c == '%' || c == '&' || c == '\''
			|| c == '*' || c == '+' || c == '`' || c == '^' || c == '|'))
			return false;
	}

	return true;
}

std::string	RequestHeaders::_toLowercase(const std::string& str) const
{
	std::string	result;
	for (size_t i = 0; i < str.size(); ++i)
		result += tolower(str[i]);

	return result;
}

bool	RequestHeaders::_isValidHeaderValue(const std::string& value)
{
	for (size_t i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		if ((c < 32 && c != '\t') || c == 127)
			return false;
	}
	
	return true;
}

bool	RequestHeaders::_isDuplicated(std::string& f_name, std::string& val)
{
	std::map<std::string, std::string>::iterator it = _headers.find(f_name);

	if (f_name == "set-cookie" || f_name == "warning")
	{
		_multiHeaders[f_name].push_back(val);
		return true;
	}

	if (it != _headers.end())
	{
		if (f_name == "host" || f_name == "content-length" || f_name == "content-type")
			return setState(false, BAD_REQUEST);
		else
			it->second += ", " + val;
	}
	else
		_headers[f_name] = val;

	return true;
}

void	RequestHeaders::clear()
{
	_raw.clear();
	_headers.clear();
	_hasHost = false;
	_statusCode = START;
	_multiHeaders.clear();
}

// TODO: Check for method, limit_except

bool	RequestHeaders::parse()
{
	std::string			line;
	size_t				headerCount = 0;
	std::istringstream	stream(_raw);

	while (std::getline(stream, line))
	{
		if (line.size() > MAX_LINE_LENGTH)
			return setState(false, HEADER_FIELDS_TOO_LARGE);
		if (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
			return setState(false, BAD_REQUEST);
		line.erase(line.size()-1);
		if (line.empty() || line == "\r")
			return setState(false, BAD_REQUEST);
		
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos || ++headerCount > MAX_HEADER)
			return setState(false, BAD_REQUEST);

		if (!checkAndStore(line, colonPos))
			return false;
	}
	if (!_hasHost)
		return setState(false, BAD_REQUEST);
	return setState(true, OK);
}


HttpStatusCode	RequestHeaders::getStatusCode() const
{
	return _statusCode;
}

bool	RequestHeaders::setState(bool tof, HttpStatusCode code)
{
	_statusCode = code;
	return tof;
}

bool	RequestHeaders::checkAndStore(std::string& line, size_t colonPos)
{
	std::string name = _trimWhitespace(line.substr(0, colonPos));
	std::string value = _trimWhitespace(line.substr(colonPos + 1));

	if (!_isValidHeaderName(name) || !_isValidHeaderValue(value))
		return setState(false, BAD_REQUEST);

	std::string lowerName = _toLowercase(name);
	if (!_isDuplicated(lowerName, value))
		return setState(false, BAD_REQUEST);

	if (lowerName == "host")
	{
		if (value.empty() || value.find(' ') != std::string::npos)
			return setState(false, BAD_REQUEST);
		_hasHost = true;
	}

	return true;
}

bool	RequestHeaders::hasHeader(const std::string& name) const
{
	std::string lowerName = _toLowercase(name);
	return _headers.find(lowerName) != _headers.end();
}

const std::map<std::string, std::string>&	RequestHeaders::getHeadersMap() const
{
	return _headers;
}

std::string	RequestHeaders::getHeaderValue(const std::string& name) const
{
	std::string lowerName = _toLowercase(name);
	std::map<std::string, std::string>::const_iterator it = _headers.find(lowerName);
	if (it != _headers.end())
		return it->second;
	
	return "";
}

const std::vector<std::string>& RequestHeaders::getMultiHeader(const std::string& name) const
{
	static const std::vector<std::string> empty;
	std::map<std::string, std::vector<std::string> >::const_iterator it = _multiHeaders.find(_toLowercase(name));
	if (it != _multiHeaders.end())
		return it->second;
	return empty;
}
