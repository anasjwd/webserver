#include "../include/ErrorResponse.hpp"
#include "../../Connection.hpp"
#include "../../conf/ErrorPage.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

static std::string getDefaultErrorFile(int statusCode) {
    std::stringstream ss;
    ss << "www/error_" << statusCode << ".html";
    return ss.str();
}

static void setDefaultErrorFile(Response& response, int statusCode) {
    std::string file = getDefaultErrorFile(statusCode);
    struct stat fileStat;
    if (stat(file.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
        response.setFilePath(file);
        response.setContentType("text/html");
        response.setFileSize(static_cast<size_t>(fileStat.st_size));
    } else {
        std::ofstream f(file.c_str());
        f << "Error " << statusCode << ": Sorry, an error (" << statusCode << ") occurred." << std::endl;
        f.close();
        if (stat(file.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            response.setFilePath(file);
            response.setContentType("text/html");
            response.setFileSize(static_cast<size_t>(fileStat.st_size));
        }
    }
}

Response ErrorResponse::createErrorResponse(int statusCode, const std::string& message) {
    (void)message;
    Response response(statusCode);
    setDefaultErrorFile(response, statusCode);
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