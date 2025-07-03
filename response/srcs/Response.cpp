#include "../include/Response.hpp"
#include <sstream>

Response::Response()
    : _statusCode(200), _statusMessage("OK") {}


void Response::setStatus(int code) {
    _statusCode = code;
    switch (code) {
        case 200: _statusMessage = "OK"; break;
        case 404: _statusMessage = "Not Found"; break;
        case 500: _statusMessage = "Internal Server Error"; break;
        default:  _statusMessage = "Unknown"; break;
    }
}

void Response::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& bodyContent) {
    _body = bodyContent;

    std::ostringstream len;
    len << _body.size();
    _headers["Content-Length"] = len.str();
}

std::string Response::build() const {
    std::ostringstream res;

    // Status line
    res << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        res << it->first << ": " << it->second << "\r\n";

    // End of headers
    res << "\r\n";

    // Body
    res << _body;

    return res.str();
}

