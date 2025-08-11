#include "../include/ReturnHandler.hpp"
#include "../include/ErrorResponse.hpp"
#include "../include/FileResponse.hpp"
#include "../../Connection.hpp"
#include <sys/socket.h>
#include <sys/stat.h>
#include <string>

Response ReturnHandler::handle(Connection* conn) {
    Return* ret = conn ? conn->getReturnDirective() : NULL;
    if (!ret) return Response();
    const Location* loc = conn->getLocation();
    std::string reqUri = conn->req->getRequestLine().getUri();
    bool applyReturn = false;
    if (loc && loc->getDirective(RETURN) == ret) {
        std::string locUri = loc->getUri() ? std::string(loc->getUri()) : "";
        if (loc->isExactMatch()) {
            if (reqUri == locUri) applyReturn = true;
        } else {
            if (!locUri.empty() && reqUri.find(locUri) == 0) applyReturn = true;
        }
    } else {
        applyReturn = true;
    }
    if (applyReturn) {
        unsigned int code = ret->getCode();
        char* url = ret->getUrl();
        std::string urlStr = url ? std::string(url) : "";
        if (!urlStr.empty() && urlStr[0] == '"' && urlStr[urlStr.size() - 1] == '"')
            urlStr = urlStr.substr(1, urlStr.size() - 2);
        if (code >= 300 && code < 400 && url && url[0]) {
           bool isAbsoluteUrl = urlStr.find("http://") == 0 || urlStr.find("https://") == 0;
            if (!urlStr.empty() && !isAbsoluteUrl && urlStr[0] != '/')
                urlStr = "/" + urlStr;

            return Response::createRedirectResponse(code, urlStr);
        } else if (code >= 400 && code < 600) {
            if (url && url[0] == '/') {
                std::string root = conn && conn->getRoot() && conn->getRoot()->getPath() ? std::string(conn->getRoot()->getPath()) : "www";
                std::string filePath = root;
                if (filePath[filePath.length() - 1] != '/') filePath += "/";
                std::string urlStr2(url);
                if (urlStr2[0] == '/') urlStr2 = urlStr2.substr(1);
                filePath += urlStr2;
                struct stat fileStat;
                if (stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                    return FileResponse::serve(filePath, "text/html", code);
                }
            }
            return ErrorResponse::createErrorResponseWithMapping(conn, code, url && url[0] ? std::string(url) : "");
        } else {
            std::string res =  Response::createErrorResponse(code,  url);
            send(conn->fd, res.c_str(), res.size(), MSG_NOSIGNAL);
            conn->fileSendState = 3;
        }
    }
    return Response();
} 