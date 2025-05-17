// # include <map>
// #include <ostream>
// # include <string>
// # include <cstddef>
// # include <sstream>
// # include <iostream>
// # include <algorithm>
// # include <sys/socket.h>
// # include "../includes/Request.hpp"

// Request::Request()
// 	: uri(""), body(""), state(0), method(""), version("")
// {
// 	std::cout << "Request created!" << std::endl;
// }

// Request::Request(const Request& req)
// {
// 	(void)req;
// }

// Request&	Request::operator=(const Request& req)
// {
// 	(void)req;
// 	return *this;
// }

// Request::~Request()
// {
// 	std::cout << "Request destroyed!" << std::endl;
// }

// static std::string	toLower(const std::string& str)
// {
// 	std::string result = str;
// 	for (size_t i = 0; i < result.size(); ++i) {
// 		result[i] = std::tolower(result[i]);
// 	}
// 	return result;
// }

// static bool	isNotSpace(unsigned char c) {
// 	return !std::isspace(c);
// }

// static void trim(std::string& str) {
// 	str.erase(str.begin(), std::find_if(str.begin(), str.end(), isNotSpace));
// 	std::string::reverse_iterator rit = std::find_if(str.rbegin(), str.rend(), isNotSpace);
// 	str.erase(rit.base(), str.end());
// }

// static bool	validChars(std::string& uri)
// {
// 	const std::string	validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

// 	if (uri.find_first_not_of(validChars) != std::string::npos)
// 		return false;

// 	return true;
// }

// bool	Request::parseUri()
// {
// 	if (uri.empty() || uri[0] != '/' || validChars(uri) == false) {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 	}

// 	std::string			pair;
// 	std::string			queryStr;

// 	size_t				queryPos = uri.find('?');
// 	if (queryPos == std::string::npos)
// 		return true;

// 	queryStr = uri.substr(queryPos + 1);
// 	std::stringstream	query(queryStr);
// 	while (std::getline(query, pair, '&')) {
// 		size_t equalPos = pair.find('=');
// 		if (equalPos != std::string::npos) {
// 			std::string key = pair.substr(0, equalPos);
// 			std::string value = pair.substr(equalPos + 1);
// 			queryParams[key] = value;
// 		}
// 	}

// 	uri = uri.substr(0, queryPos);
// 	if (uri.length() > 2048) {
// 		state = HTTP_URI_TOO_LONG;
// 		return false;
// 	}

// 	return true;
// }

// bool	Request::parseMethod()
// {
// 	if (method != "GET" && method != "POST" && method != "DELETE") {
// 		state = HTTP_METHOD_NOT_ALLOWED;
// 		return false;
// 	}

// 	return true;
// }

// bool	Request::parseVersion()
// {
// 	if (version.size() < 8 || version.substr(0, 5) != "HTTP/") {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 	}

// 	std::string	vsn = version.substr(5);

// 	if (vsn.length() != 3 && vsn != "1.1") {
// 		state = HTTP_VERSION_NOT_SUPPORTED;
// 		return false;
// 	}

// 	return true;
// }

// bool	Request::parseHeaders(std::stringstream& stream, std::string& line)
// {
// 	while (std::getline(stream, line)) {
// 		if (line.empty() || line == "\r")
// 		break;
	
// 	size_t colonPos = line.find(':');
// 	if (colonPos == std::string::npos) {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 		}
		
// 		std::string key = line.substr(0, colonPos);
// 		std::string value = line.substr(colonPos + 1);
// 		trim(key);
// 		trim(value);
		
// 		if (key.empty()) {
// 			state = HTTP_BAD_REQUEST;
// 			return false;
// 		}
		
// 		headers[toLower(key)] = value;
// 	}
	
// 	return true;
// }

// void	Request::clearRequest()
// {
// 	uri = "";
// 	body = "";
// 	state = 0;
// 	method = "";
// 	version = "";
// 	headers.clear();
// 	bodyParams.clear();
// 	queryParams.clear();
// }

// bool    Request::parseUrlEncodedBody()
// {
// 	std::string			pair;
// 	std::stringstream	ss(body);

// 	while (std::getline(ss, pair, '&')) {
// 		size_t equalPos = pair.find('=');
// 		if (equalPos != std::string::npos) {
// 			std::string key = pair.substr(0, equalPos);
// 			std::string value = pair.substr(equalPos + 1);
// 			bodyParams[key] = value;
// 		}
// 	}

// 	return true;
// }

// bool	Request::parseBodyByContentType()
// {
// 	std::string	contentType;

// 	if (headers.find("content-type") != headers.end())
// 		contentType = headers["content-type"];
// 	else
// 	 	return true;

// 	/*
// 		To be done !
// 	*/

// 	return false;
// }

// bool	Request::parseRequestLineAndHeaders(std::string& buffer)
// {
// 	size_t pos = buffer.find("\r\n\r\n");
// 	if (pos == std::string::npos)
// 		return false;

// 	std::string			firstLine;
// 	std::string			headerPart = buffer.substr(0, pos + 2);
// 	std::stringstream	stream(headerPart);

// 	if (!std::getline(stream, firstLine) || firstLine.empty()) {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 	}

// 	std::string			tmp;
// 	std::stringstream	lineStream(firstLine);
// 	if (!(lineStream >> method >> uri >> version) || lineStream >> tmp) {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 	}

// 	if (!parseMethod() || !parseUri() || !parseVersion())
// 		return false;

// 	std::string			line;
// 	if (!parseHeaders(stream, line))
// 		return false;

// 	buffer = buffer.substr(pos + 4);
// 	return true;
// }

// bool	Request::readBodyFromBuffer(std::string& buffer)
// {
// 	size_t	contentLength = 0;
// 	std::map<std::string, std::string>::iterator	it = headers.find("content-length");

// 	if (it == headers.end() && method == "POST") {
// 		state = HTTP_LENGTH_REQUIRED;
// 		return false;
// 	}

// 	char*	endptr;
// 	contentLength = strtoul(it->second.c_str(), &endptr, 10);
// 	if (*endptr != '\0' || endptr == it->second.c_str()) {
// 		state = HTTP_BAD_REQUEST;
// 		return false;
// 	}

// 	if (buffer.size() < contentLength)
// 		return false;

// 	body = buffer.substr(0, contentLength);
// 	std::stringstream	bodyStream(body);

// 	if (!parseBodyByContentType())
// 		return false;

// 	return true;
// }

// bool	Request::readFromSocket(int socketfd)
// {
// 	clearRequest();

// 	std::string		buffer;
// 	char			chunk[1024];
// 	bool			headersParsed = false;
// 	size_t			bytesRead = recv(socketfd, chunk, sizeof(chunk), 0);
	
// 	if (bytesRead < 0) {
// 		state = HTTP_INTERNAL_SERVER_ERROR;
// 		return false;
// 	}

// 	if (bytesRead == 0)
// 		return true;

// 	buffer.append(chunk, bytesRead);

// 	if (!headersParsed) {
// 		if (!parseRequestLineAndHeaders(buffer)) {
// 			if (state != 0)
// 				return false;
// 			return false;
// 		}
// 		headersParsed = true;
// 	}

// 	if (headersParsed && method == "POST") {
// 		if (!readBodyFromBuffer(buffer))
// 			if (state != 0)
// 				return false;
// 	}

// 	state = HTTP_OK;
// 	return true;
// }

// /*
// 	Each connection should have:
// 		* It own buffer (make it a file maybe).
// 		* A specific chunk size, a default one and a custom one maybe from the conf file.
// 		* A boolean to check if the headers are already parsed or not, set to false by default.
// 		* A boolean that check either the whole request is finished.
// */

// std::ostream&	operator<<(std::ostream& os, const Request& req)
// {
// 	os << "Request query:\n"
// 		<< "Method: " << req.method << ", Uri: " << req.uri
// 		<< ", Version: " << req.version << "." << std::endl;

// 	if (!req.queryParams.empty()) {
// 		os << "Query Parameters:\n";
// 		std::map<std::string, std::string>::const_iterator it = req.queryParams.begin();
// 		for (;it != req.queryParams.end(); ++it)
// 			os << "\t" << it->first << " = " << it->second << "\n";
// 	}
	
// 	if (!req.headers.empty()) {
// 		os << "Request Headers:\n";
// 		std::map<std::string, std::string>::const_iterator it = req.headers.begin();
// 		for (;it != req.headers.end(); ++it)
// 			os << "\t" << it->first << ": " << it->second << "\n";
// 	}

// 	if (req.method == "POST")
// 	{
// 		os << req.body;
// 	}
// 	// Print body when fully parsed!

// 	os << std::endl;
// 	return os;
// }

///////////////////////////////////////

#include <fcntl.h>
#include <ostream>
#include <cstddef>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <ctime>
#include "../includes/Request.hpp"

Request::Request()
    : uri(""), body(""), state(0), method(""), version("") {}

Request::Request(const Request& req) { (void)req; }

Request& Request::operator=(const Request& req) {
    (void)req;
    return *this;
}

Request::~Request() {}

static std::string toLower(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

static bool isNotSpace(unsigned char c) {
    return !std::isspace(c);
}

static void trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), isNotSpace));
    std::string::reverse_iterator rit = std::find_if(str.rbegin(), str.rend(), isNotSpace);
    str.erase(rit.base(), str.end());
}

static bool validChars(std::string& uri) {
    const std::string validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    return uri.find_first_not_of(validChars) == std::string::npos;
}

bool Request::parseUri() {
    if (uri.empty() || uri[0] != '/' || !validChars(uri)) {
        state = HTTP_BAD_REQUEST;
        return false;
    }

    size_t pos;
    while ((pos = uri.find("../")) != std::string::npos) {
        uri.erase(pos, 3);
    }

    size_t queryPos = uri.find('?');
    if (queryPos == std::string::npos)
        return true;

    std::string queryStr = uri.substr(queryPos + 1);
    std::stringstream query(queryStr);
    std::string pair;

    while (std::getline(query, pair, '&')) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            queryParams[key] = value;
        }
    }

    uri = uri.substr(0, queryPos);
    if (uri.length() > MAX_URI_LENGTH) {
        state = HTTP_URI_TOO_LONG;
        return false;
    }

    return true;
}

bool Request::parseMethod() {
    if (method != "GET" && method != "POST" && method != "DELETE") {
        state = HTTP_METHOD_NOT_ALLOWED;
        return false;
    }
    return true;
}

bool Request::parseVersion() {
    if (version.size() < 8 || version.substr(0, 5) != "HTTP/") {
        state = HTTP_BAD_REQUEST;
        return false;
    }

    std::string vsn = version.substr(5);
    if (vsn.length() != 3 || vsn != "1.1") {
        state = HTTP_VERSION_NOT_SUPPORTED;
        return false;
    }
    return true;
}

bool Request::parseHeaders(std::stringstream& stream, std::string& line) {
    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") break;
        
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

void Request::clearRequest() {
    uri.clear();
    body.clear();
    state = 0;
    method.clear();
    version.clear();
    headers.clear();
    bodyParams.clear();
    queryParams.clear();
}

bool Request::parseJsonBody() {
    if (body.empty() || (body[0] != '{' && body[0] != '[')) {
        state = HTTP_BAD_REQUEST;
        return false;
    }
    bodyParams["_raw_json"] = body;
    return true;
}

bool Request::parseMultipartBody() {
    std::string contentType = headers["content-type"];
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        state = HTTP_BAD_REQUEST;
        return false;
    }

    std::string boundary = "--" + contentType.substr(boundaryPos + 9);
    size_t pos = 0;

    while (pos < body.size()) {
    	std::cout << "Multipart parsing!\n\n";
        size_t partStart = body.find(boundary, pos);
        if (partStart == std::string::npos) break;
        
        size_t partEnd = body.find(boundary, partStart + boundary.length());
        if (partEnd == std::string::npos) partEnd = body.size();

        std::string part = body.substr(partStart + boundary.length(), 
                                     partEnd - (partStart + boundary.length()));
        
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            state = HTTP_BAD_REQUEST;
            return false;
        }
        std::cout <<"PartHeader ====> \n";
        std::string partHeaders = part.substr(0, headerEnd);
        std::cout <<"\t " << partHeaders << "PartBody ====> \n";
        std::string partBody = part.substr(headerEnd + 4);
        std::cout <<"\t " << partBody << "\n";

        size_t dispPos = partHeaders.find("Content-Disposition:");
        if (dispPos != std::string::npos) {
            size_t namePos = partHeaders.find("name=\"", dispPos);
            if (namePos != std::string::npos) {
                namePos += 6;
                size_t nameEnd = partHeaders.find("\"", namePos);
                // std::string name = partHeaders.substr(namePos, nameEnd - namePos);
                std::cout << "nameCalc: " << nameEnd - namePos;
                // std::cout << "value: " << partBody;
                // bodyParams[name] = partBody;
            }
        }

        pos = partEnd;
    }
    return true;
}

bool Request::parseUrlEncodedBody() {
    std::string pair;
    std::stringstream ss(body);

    while (std::getline(ss, pair, '&')) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            bodyParams[key] = value;
        }
    }

    state = HTTP_OK;
    return true;
}

bool Request::parseBodyByContentType() {
    std::string contentType;
    std::map<std::string, std::string>::iterator it = headers.find("content-type");
    if (it != headers.end())
        contentType = it->second;
    else
        return true;

    size_t semiColon = contentType.find(';');
    if (semiColon != std::string::npos)
        contentType = contentType.substr(0, semiColon);

	std::cout << "Content type: " << contentType << "\n\n";

    if (contentType == "application/x-www-form-urlencoded")
        return parseUrlEncodedBody(); // Seems good.
    else if (contentType == "multipart/form-data")
        return parseMultipartBody(); // Working on it, now!
    else if (contentType == "application/json")
        return parseJsonBody();
    
    state = HTTP_UNSUPPORTED_MEDIA_TYPE;
    return false;
}

bool	Request::parseRequestLineAndHeaders(std::string& buffer) {
    size_t pos = buffer.find("\r\n\r\n");
    if (pos == std::string::npos)
        return false;

    std::string firstLine;
    std::string headerPart = buffer.substr(0, pos + 2);
    std::stringstream stream(headerPart);

    if (!std::getline(stream, firstLine) || firstLine.empty()) {
        state = HTTP_BAD_REQUEST;
        return false;
    }

    std::string tmp;
    std::stringstream lineStream(firstLine);
    if (!(lineStream >> method >> uri >> version) || lineStream >> tmp) {
        state = HTTP_BAD_REQUEST;
        return false;
    }

    if (!parseMethod() || !parseUri() || !parseVersion())
        return false;

    std::string line;
    if (!parseHeaders(stream, line))
        return false;

    buffer = buffer.substr(pos + 4);
    return true;
}

bool Request::readBodyFromBuffer(std::string& buffer) {
    size_t contentLength = 0;
    std::map<std::string, std::string>::iterator it = headers.find("content-length");

    if (it == headers.end() && method == "POST") {
        state = HTTP_LENGTH_REQUIRED;
        return false;
    }

    char* endptr;
    contentLength = strtoul(it->second.c_str(), &endptr, 10);
    if (*endptr != '\0' || endptr == it->second.c_str()) {
        state = HTTP_BAD_REQUEST;
        return false;
    }
    std::cout << "Body size: " << buffer.size() << ", Expected: " << contentLength << "\n";

    // MAX_BODY_SIZE should be set from conf file, or use the default one.
    if (contentLength > MAX_BODY_SIZE)
    {
        state = HTTP_PAYLOAD_TOO_LARGE;
        return false;
    }
	std::cout << "Arrivd here!\n\n"
		<< "0=> " << it->second.c_str()
		<< "\n1=> " << buffer.size()
		<< "\n2=> " << contentLength << "\n"
        << "buff => \n " << buffer << "\n";

    if (buffer.size() < contentLength) {
        state = HTTP_INCOMPLETE;
        return false;
    }

    body = buffer.substr(0, contentLength);
    if (!parseBodyByContentType())
        return false;

    buffer = buffer.substr(contentLength);
    return true;
}

bool Request::parseFromBuffer(std::string& buffer) {
    clearRequest();
    if (!parseRequestLineAndHeaders(buffer))
        return false;

    if (method == "POST") {
        if (!readBodyFromBuffer(buffer)) {
            if (state != HTTP_INCOMPLETE)
                return false;
            return false;
        }
    }

    state = HTTP_OK;
    return true;
}

std::string getStatusMessage(int statusCode) {
    switch (statusCode) {
        case 200:
			return "OK";
        case 400:
			return "Bad Request";
        case 403:
			return "Forbidden";
        case 404:
			return "Not Found";
        case 405:
			return "Method Not Allowed";
        case 411:
			return "Length Required";
        case 413:
			return "Payload Too Large";
        case 414:
			return "URI Too Long";
        case 415:
			return "Unsupported Media Type";
        case 500:
			return "Internal Server Error";
        case 505:
			return "HTTP Version Not Supported";
        default:
			return "Unknown Status";
    }
}

std::string Request::getContentType()
{
    std::string contentType = "text/html"; // default
    size_t dotPos = uri.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = uri.substr(dotPos);
        if (extension == ".css")
            contentType = "text/css";
        else if (extension == ".js")
            contentType = "application/javascript";
        else if (extension == ".png")
            contentType = "image/png";
        else if (extension == ".jpg" || extension == ".jpeg")
            contentType = "image/jpeg";
        else if (extension == ".gif")
            contentType = "image/gif";
        else if (extension == ".svg")
            contentType = "image/svg+xml";
        else if (extension == ".ico")
            contentType = "image/x-icon";
        else if (extension == ".json")
            contentType = "application/json";
        else if (extension == ".txt")
            contentType = "text/plain";
    }
    return contentType;
}

void Request::sendResponse(int client_fd)
{
    std::string filePath = "www" + uri;
    if (uri == "/" || uri.empty())
        filePath = "www/index.html";

    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::string errorRes = version + " 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(client_fd, errorRes.c_str(), errorRes.size(), 0);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string body = buffer.str();

    std::string type = getContentType();

    std::ostringstream response;
    response << version << " " << state << " " << getStatusMessage(state) << "\r\n"
             << "Content-Type: " << type << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

    std::string responseStr = response.str();
    send(client_fd, responseStr.c_str(), responseStr.size(), 0);
}

std::ostream& operator<<(std::ostream& os, const Request& req) {
    os << "Request query:\n"
       << "Method: " << req.method << ", Uri: " << req.uri
       << ", Version: " << req.version << "\nState: " << req.state << "\n";

    if (!req.queryParams.empty()) {
        os << "Query Parameters:\n";
        for (std::map<std::string, std::string>::const_iterator it = req.queryParams.begin();
             it != req.queryParams.end(); ++it) {
            os << "\t" << it->first << " = " << it->second << "\n";
        }
    }
    
    if (!req.headers.empty()) {
        os << "Request Headers:\n";
        for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();
             it != req.headers.end(); ++it) {
            os << "\t" << it->first << ": " << it->second << "\n";
        }
    }

    if (req.method == "POST") {
        os << "Body (raw): '" << req.body << "'\n";
        if (!req.bodyParams.empty()) {
            os << "Parsed Body Parameters:\n";
            for (std::map<std::string, std::string>::const_iterator it = req.bodyParams.begin();
                 it != req.bodyParams.end(); ++it) {
                os << "\t" << it->first << " = " << it->second << "\n";
            }
        }
    }

    return os;
}
