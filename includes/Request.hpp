#pragma once

#include <map>
#include <string>

#define HTTP_INCOMPLETE                0
#define HTTP_OK                        200
#define HTTP_BAD_REQUEST               400
#define HTTP_UNAUTHORIZED              401
#define HTTP_FORBIDDEN                 403
#define HTTP_NOT_FOUND                 404
#define HTTP_METHOD_NOT_ALLOWED        405
#define HTTP_LENGTH_REQUIRED           411
#define HTTP_PAYLOAD_TOO_LARGE         413
#define HTTP_URI_TOO_LONG              414
#define HTTP_UNSUPPORTED_MEDIA_TYPE    415
#define HTTP_INTERNAL_SERVER_ERROR     500
#define HTTP_VERSION_NOT_SUPPORTED     505

#define MAX_BODY_SIZE                 (10 * 1024 * 1024) // 10MB
#define MAX_URI_LENGTH                2048

class Request {
private:
    Request(const Request& req);
    Request& operator=(const Request& req);

public:
    Request();
    ~Request();

    std::string                         uri;
    std::string                         body;
    int                                 state;
    std::string                         method;
    std::string                         version;

    std::map<std::string, std::string>  headers;
    std::map<std::string, std::string>  bodyParams;
    std::map<std::string, std::string>  queryParams;

    bool                                parseUri();
    bool                                parseMethod();
    bool                                parseVersion();
    bool                                parseHeaders(std::stringstream& stream, std::string& line);

    void                                clearRequest();

    bool                                parseJsonBody();
    bool                                parseMultipartBody();
    bool                                parseUrlEncodedBody();
    bool                                parseBodyByContentType();

    void                                sendResponse(int client_fd);
    
    bool                                parseFromBuffer(std::string& buffer);
    bool                                readBodyFromBuffer(std::string& buffer);
    bool                                parseRequestLineAndHeaders(std::string& buffer);

    //////////////////////
	std::string							getContentType();
};

std::ostream& operator<<(std::ostream& os, const Request& req);
