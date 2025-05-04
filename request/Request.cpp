# include <map>
# include <string>
# include <cstddef>
# include <sstream>
# include <iostream>
# include <algorithm>
# include <sys/socket.h>
# include "../includes/Request.hpp"

Request::Request()
	:   uri(""), body(""), method(""), version(""), state(0), close(0)
{
	std::cout << "Request created!" << std::endl;
}

Request::~Request()
{
	std::cout << "Request destroyed!" << std::endl;
}

static std::string toLower(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.size(); ++i) {
		result[i] = std::tolower(result[i]);
	}
	return result;
}

static void	trim(std::string& str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) {
		return !std::isspace(c);
	}));

	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
		return !std::isspace(c);
	}).base(), str.end());
}

static	bool	validChars(std::string& uri)
{
	const std::string	validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

	if (uri.find_first_not_of(validChars) != std::string::npos)
		return false;

	return true;
}

bool	Request::parseUri()
{
	// Check if empty or invalid URI.
	if (uri.empty() || uri[0] != '/' || validChars(uri) == false) {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	std::string			pair;
	std::string			queryStr;

	// Check if there's query in the URI.
	size_t				queryPos = uri.find('?');
	if (queryPos == std::string::npos)
		return true;

	// Read and store queries.
	queryStr = uri.substr(queryPos + 1);
	std::stringstream	query(queryStr);
	while (std::getline(query, pair, '&')) {
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos) {
			std::string key = pair.substr(0, equalPos);
			std::string value = pair.substr(equalPos + 1);
			queryParams[key] = value;
		}
	}

	// Check URI length.
	uri = uri.substr(0, queryPos);
	if (uri.length() > 2048) {
		state = HTTP_URI_TOO_LONG;
		return false;
	}

	return true;
}

bool	Request::parseMethod()
{
	// Validate methods.
	if (method != "GET" && method != "POST" && method != "DELETE") {
		state = HTTP_METHOD_NOT_ALLOWED;
		return false;
	}

	return true;
}

bool	Request::parseVersion()
{
	// Check used protocol and version of it.
	if (version.size() < 8 || version.substr(0, 5) != "HTTP/") {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	std::string	vsn = version.substr(5);

	// Supporting HTTP/1.1 only.
	if (vsn != "1.1") {
		state = HTTP_VERSION_NOT_SUPPORTED;
		return false;
	}

	return true;
}

bool	Request::parseHeaders(std::stringstream& stream, std::string& line)
{
	while (std::getline(stream, line)) {
		// Break if empty line or headers are finished.
		if (line.empty() || line == "\r")
			break;

		// Check for ':' in the readed header line.
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			state = HTTP_BAD_REQUEST;
			return false;
		}

		// Seperate key, value and trim them.
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		trim(key);
		trim(value);
		
		if (key.empty()) {
			state = HTTP_BAD_REQUEST;
			return false;
		}
		// Save the parsed header.
		headers[toLower(key)] = value;
	}

	return true;
}

bool    Request::parseUrlEncodedBody()
{
	// Not done yet!
	std::string			pair;
	std::stringstream	ss(body);

	while (std::getline(ss, pair, '&')) {
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos) {
			std::string key = pair.substr(0, equalPos);
			std::string value = pair.substr(equalPos + 1);
			bodyParams[key] = value;
		}
	}

	return true;
}


bool	Request::parseBodyByContentType()
{
	// Not done yet!
	std::string	contentType;

	if (headers.find("content-type") != headers.end())
		contentType = headers["content-type"];
	else
		return true;

	if (contentType == "application/x-www-form-urlencoded")
		return parseUrlEncodedBody();
	else if (contentType.find("multipart/form-data") == 0)
		return parseMultipartBody(contentType);
	else if (contentType == "application/json")
		return parseJsonBody();
	else if (contentType == "text/plain")
		return true;
	else
		state = HTTP_UNSUPPORTED_MEDIA_TYPE;

	return false;
}

void    Request::clearRequest()
{
	// Clear the request!
	uri = "";
	body = "";
	state = 0;
	method = "";
	version = "";
	headers.clear();
	bodyParams.clear();
	queryParams.clear();
}

bool	Request::parseRequestLineAndHeaders(std::string& buffer)
{
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;

	std::string			firstLine;
	std::string			headerPart = buffer.substr(0, pos + 2);
	std::stringstream	stream(headerPart);

	if (!std::getline(stream, firstLine) || firstLine.empty()) {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	std::string			tmp;
	std::stringstream	lineStream(firstLine);
	if (!(lineStream >> method >> uri >> version) || lineStream >> tmp) {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	if (!parseMethod() || !parseUri() || !parseVersion())
		return false;

	std::string			line;
	if (!parseHeaders(stream, line))
		return false;

	buffer = buffer.substr(pos + 4);
	return true;
}

bool	Request::readBodyFromBuffer(std::string& buffer)
{
	size_t	contentLength = 0;
	std::map<std::string, std::string>::iterator	it = headers.find("content-length");

	if (it == headers.end() && method == "POST") {
		state = HTTP_LENGTH_REQUIRED;
		return false;
	}

	char*	endptr;
	contentLength = strtoul(it->second.c_str(), &endptr, 10);
	if (*endptr != '\0' || endptr == it->second.c_str()) {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	if (buffer.size() < contentLength)
		return false;

	body = buffer.substr(0, contentLength);
	std::stringstream	bodyStream(body);

	if (!parseBodyByContentType())
		return false;

	return true;
}

void	Request::readFromSocket(int socketfd)
{
	clearRequest();

	std::string		buffer;
	ssize_t			bytesRead;
	char			chunk[1024];
	bool			headersParsed = false;

	while (1) {
		bytesRead = recv(socketfd, chunk, sizeof(chunk), 0);
		if (bytesRead < 0) {
			state = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}
		if (bytesRead == 0)
			break;
		buffer.append(chunk, bytesRead);
		if (!headersParsed) {
			if (!parseRequestLineAndHeaders(buffer)) {
				if (state != 0)
					break;
				continue;
			}
			headersParsed = true;
			if (method == "GET" || method == "DELETE") {
				body.clear();
				state = HTTP_OK;
				break;
			}
		}

		if (headersParsed && method == "POST") {
			if (readBodyFromBuffer(buffer)) {
				state = HTTP_OK;
				break;
			}
		}
	}
}
