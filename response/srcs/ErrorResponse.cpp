#include "../include/ErrorResponse.hpp"
#include "../../Connection.hpp"
#include "../../conf/ErrorPage.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>




Response ErrorResponse::createErrorResponse(int statusCode, const std::string& message) {
    (void)message;
    Response response(statusCode);
    response.setContentType("text/html");
    std::ostringstream error;
    error << "/tmp/error";
    std::string errorPath = error.str();
    std::ofstream tempFile(errorPath.c_str());
    if (tempFile.is_open()) {
        tempFile << "<center>    <h1>ERROR ";
        tempFile << statusCode;
        tempFile << " </h1><hr> <p><em>Webserv/1.1</em></p></center>";
    }
    response.setFileBody(errorPath);
    response.setFileSize(72);
    return response;
}

Response ErrorResponse::createErrorResponseWithMapping(Connection* conn, int statusCode, const std::string& message) {
    if (conn) {
        ErrorPage* ep = conn->getErrorPageForCode(statusCode);
        if (ep && ep->getUri() && ep->getUri()[0]) {
            std::string root = conn->getRoot() && conn->getRoot()->getPath() ? std::string(conn->getRoot()->getPath()) : "www";
            std::string errorPath = root;
            if (errorPath[errorPath.length() - 1] != '/') errorPath += "/";
            std::string uriStr(ep->getUri());
            if (uriStr[0] == '/') uriStr = uriStr.substr(1);
            errorPath += uriStr;
            struct stat fileStat;
            if (stat(errorPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                Response response(statusCode);
                response.setFilePath(errorPath);
                response.setContentType("text/html");
                response.setFileSize(static_cast<size_t>(fileStat.st_size));
                return response;
            }
        }
    }
    return createErrorResponse(statusCode, message);
}

Response ErrorResponse::createMethodNotAllowedResponse(Connection* conn, const std::vector<std::string>& allowedMethods) {
    std::string methods;
    for (size_t i = 0; i < allowedMethods.size(); ++i) {
        if (i > 0) methods += ", ";
        methods += allowedMethods[i];
    }
    Response errorBodyResponse = createErrorResponseWithMapping(conn, 405);
    errorBodyResponse.addHeader("Allow", methods);
    return errorBodyResponse;
}

Response ErrorResponse::createNotFoundResponse(Connection* conn) {
    return createErrorResponseWithMapping(conn, 404, "Not Found");
}
Response ErrorResponse::createForbiddenResponse(Connection* conn) {
    return createErrorResponseWithMapping(conn , 403, "Forbidden");
}
Response ErrorResponse::createInternalErrorResponse(Connection* conn) {
    return createErrorResponseWithMapping(conn ,500, "Internal Server Error");
} 