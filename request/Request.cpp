# include <map>
#include <ostream>
# include <string>
# include <cstddef>
# include <sstream>
# include <iostream>
# include <algorithm>
# include <sys/socket.h>
# include "../includes/Request.hpp"

Request::Request()
	: uri(""), body(""), state(0), method(""), version("")
{
	std::cout << "Request created!" << std::endl;
}

Request::Request(const Request& req)
{
	(void)req;
}

Request&	Request::operator=(const Request& req)
{
	(void)req;
	return *this;
}

Request::~Request()
{
	std::cout << "Request destroyed!" << std::endl;
}

static std::string	toLower(const std::string& str)
{
	std::string result = str;
	for (size_t i = 0; i < result.size(); ++i) {
		result[i] = std::tolower(result[i]);
	}
	return result;
}

static bool	isNotSpace(unsigned char c) {
	return !std::isspace(c);
}

static void trim(std::string& str) {
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), isNotSpace));
	std::string::reverse_iterator rit = std::find_if(str.rbegin(), str.rend(), isNotSpace);
	str.erase(rit.base(), str.end());
}

static bool	validChars(std::string& uri)
{
	const std::string	validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

	if (uri.find_first_not_of(validChars) != std::string::npos)
		return false;

	return true;
}

bool	Request::parseUri()
{
	if (uri.empty() || uri[0] != '/' || validChars(uri) == false) {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	std::string			pair;
	std::string			queryStr;

	size_t				queryPos = uri.find('?');
	if (queryPos == std::string::npos)
		return true;

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

	uri = uri.substr(0, queryPos);
	if (uri.length() > 2048) {
		state = HTTP_URI_TOO_LONG;
		return false;
	}

	return true;
}

bool	Request::parseMethod()
{
	if (method != "GET" && method != "POST" && method != "DELETE") {
		state = HTTP_METHOD_NOT_ALLOWED;
		return false;
	}

	return true;
}

bool	Request::parseVersion()
{
	if (version.size() < 8 || version.substr(0, 5) != "HTTP/") {
		state = HTTP_BAD_REQUEST;
		return false;
	}

	std::string	vsn = version.substr(5);

	if (vsn.length() != 3 && vsn != "1.1") {
		state = HTTP_VERSION_NOT_SUPPORTED;
		return false;
	}

	return true;
}

bool	Request::parseHeaders(std::stringstream& stream, std::string& line)
{
	while (std::getline(stream, line)) {
		if (line.empty() || line == "\r")
		break;
	
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos) {
		state = HTTP_BAD_REQUEST;
		return false;
		}
		
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		trim(key);
		trim(value);
		
		if (key.empty()) {
			state = HTTP_BAD_REQUEST;
			return false;
		}
		
		headers[toLower(key)] = value;
	}
	
	return true;
}

void	Request::clearRequest()
{
	uri = "";
	body = "";
	state = 0;
	method = "";
	version = "";
	headers.clear();
	bodyParams.clear();
	queryParams.clear();
}

bool    Request::parseUrlEncodedBody()
{
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
	std::string	contentType;

	if (headers.find("content-type") != headers.end())
		contentType = headers["content-type"];
	else
	 	return true;

	/*
		To be done !
	*/

	return false;
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

bool	Request::readFromSocket(int socketfd)
{
	clearRequest();

	std::string		buffer;
	char			chunk[1024];
	bool			headersParsed = false;
	size_t			bytesRead = recv(socketfd, chunk, sizeof(chunk), 0);
	
	if (bytesRead < 0) {
		state = HTTP_INTERNAL_SERVER_ERROR;
		return false;
	}

	if (bytesRead == 0)
		return true;

	buffer.append(chunk, bytesRead);

	if (!headersParsed) {
		if (!parseRequestLineAndHeaders(buffer)) {
			if (state != 0)
				return false;
			return false;
		}
		headersParsed = true;
	}

	if (headersParsed && method == "POST") {
		if (!readBodyFromBuffer(buffer))
			if (state != 0)
				return false;
	}

	state = HTTP_OK;
	return true;
}

/*
	Each connection should have:
		* It own buffer (make it a file maybe).
		* A specific chunk size, a default one and a custom one maybe from the conf file.
		* A boolean to check if the headers are already parsed or not, set to false by default.
		* A boolean that check either the whole request is finished.
*/

std::ostream&	operator<<(std::ostream& os, const Request& req)
{
	os << "Request query:\n"
		<< "Method: " << req.method << ", Uri: " << req.uri
		<< ", Version: " << req.version << "." << std::endl;

	if (!req.queryParams.empty()) {
		os << "Query Parameters:\n";
		std::map<std::string, std::string>::const_iterator it = req.queryParams.begin();
		for (;it != req.queryParams.end(); ++it)
			os << "\t" << it->first << " = " << it->second << "\n";
	}
	
	if (!req.headers.empty()) {
		os << "Request Headers:\n";
		std::map<std::string, std::string>::const_iterator it = req.headers.begin();
		for (;it != req.headers.end(); ++it)
			os << "\t" << it->first << ": " << it->second << "\n";
	}

	if (req.method == "POST")
	{
		os << req.body;
	}
	// Print body when fully parsed!

	os << std::endl;
	return os;
}
