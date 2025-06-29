#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"
#include "utils.hpp"

// Forward declarations
class Request;
class Server;
class Location;

class ResponseHandler {
private:
    std::string _rootPath;
    std::string _uploadPath;
    std::string _cgiPath;
    unsigned int _maxBodySize;
    bool _autoIndex;
    std::map<int, std::string> _errorPages;
    std::map<std::string, std::string> _mimeTypes;
    
    // Helper methods
    std::string _getServerRoot(const Server* server) const;
    std::string _getLocationRoot(const Location* location) const;
    unsigned int _getMaxBodySize(const Server* server) const;
    bool _isAllowedMethod(const std::string& method, const Location* location) const;
    std::string _buildFilePath(const std::string& uri, const std::string& root) const;
    std::string _getErrorPage(int statusCode) const;
    std::string _executeCGI(const std::string& scriptPath, const Request& request) const;
    std::string _handleFileUpload(const Request& request) const;
    bool _deleteFile(const std::string& filePath) const;
    std::string _generateDirectoryListing(const std::string& path, const std::string& uri) const;
    std::string _getMimeType(const std::string& path) const;
    bool _isCGIScript(const std::string& path) const;
    std::string _urlDecode(const std::string& str) const;
    std::string _urlEncode(const std::string& str) const;
    
    // HTTP method handlers
    Response _handleGET(const Request& request, const Server* server, const Location* location) const;
    Response _handlePOST(const Request& request, const Server* server, const Location* location) const;
    Response _handleDELETE(const Request& request, const Server* server, const Location* location) const;
    
public:
    ResponseHandler();
    ~ResponseHandler();
    
    Response handleRequest(const Request& request, const Server* server, const Location* location = NULL);
    void setErrorPages(const std::map<int, std::string>& errorPages);
    void setAutoIndex(bool autoIndex);
}; 