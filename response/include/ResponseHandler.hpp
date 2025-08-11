#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"


class Request;
class Server;
class Location;
class Connection;

class ResponseHandler {
public:

    static Response handleRequest(Connection* conn);
    static Response createErrorResponse(int statusCode, const std::string& message);
    static Response createMethodNotAllowedResponse(Connection* conn, const std::vector<std::string>& allowedMethods);
    static Response createNotFoundResponse(Connection* conn);
    static Response createForbiddenResponse();
    static Response createInternalErrorResponse();
    static Response createErrorResponseWithMapping(Connection* conn, int statusCode, const std::string& message);
    static std::string _getRootPath(Connection* conn);
    static std::string _buildFilePath(const std::string& uri, const std::string& root,  const Location* location);

private:
    static std::map<std::string, std::string> _mimeTypes;
    static bool _getAutoIndex(Connection* conn);
    static std::map<int, std::string> _getErrorPages(Connection* conn);
    static std::vector<std::string> _getIndexFiles(Connection* conn);
    static std::string _getMimeType(const std::string& path);
    static std::string _generateDirectoryListing(const std::string& path, const std::string& uri);
}; 