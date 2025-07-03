#include "../include/ResponseHandler.hpp"
#include "../../request/incs/Request.hpp"
#include "../../request/incs/RequestLine.hpp"
#include "../../request/incs/RequestHeaders.hpp"
#include "../../request/incs/RequestBody.hpp"
#include "../../conf/Server.hpp"
#include "../../conf/Location.hpp"
#include "../../conf/Root.hpp"
#include "../../conf/ClientMaxBodySize.hpp"
#include "../../conf/Index.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <algorithm>
#include <fcntl.h>
#include <sys/wait.h>
#include <cctype>

// Helper function for toString
static std::string toString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

static std::string timeToString(time_t time) {
    std::stringstream ss;
    ss << time;
    return ss.str();
}

ResponseHandler::ResponseHandler() 
    : _rootPath("www"), _uploadPath("uploads"), _cgiPath("cgi-bin"), 
      _maxBodySize(1024 * 1024), _autoIndex(false) {
    
    // Initialize MIME types
    _mimeTypes[".html"] = "text/html";
    _mimeTypes[".htm"] = "text/html";
    _mimeTypes[".css"] = "text/css";
    _mimeTypes[".js"] = "application/javascript";
    _mimeTypes[".png"] = "image/png";
    _mimeTypes[".jpg"] = "image/jpeg";
    _mimeTypes[".jpeg"] = "image/jpeg";
    _mimeTypes[".gif"] = "image/gif";
    _mimeTypes[".ico"] = "image/x-icon";
    _mimeTypes[".txt"] = "text/plain";
    _mimeTypes[".pdf"] = "application/pdf";
    _mimeTypes[".py"] = "text/plain";
}

ResponseHandler::~ResponseHandler() {}

Response ResponseHandler::handleRequest(const Request& request, const Server* server, const Location* location) {
    Response response;
    std::string method = request.getRequestLine().getMethod();
    std::string uri = request.getRequestLine().getUri();
    
    // Check if method is allowed
    if (!_isAllowedMethod(method, location)) {
        response.setStatus(405);
        response.setBody(_getErrorPage(405));
        response.addHeader("Content-Type", "text/html");
        response.addHeader("Allow", "GET, POST, DELETE");
        return response;
    }
    
    // Check body size for POST requests
    if (method == "POST") {
        unsigned int maxSize = _getMaxBodySize(server);
        if (request.getRequestBody().getContentLength() > maxSize) {
            response.setStatus(413);
            response.setBody(_getErrorPage(413));
            response.addHeader("Content-Type", "text/html");
            return response;
        }
    }
    
    // Handle different HTTP methods
    if (method == "GET") {
        return _handleGET(request, server, location);
    } else if (method == "POST") {
        return _handlePOST(request, server, location);
    } else if (method == "DELETE") {
        return _handleDELETE(request, server, location);
    }
    
    // Method not implemented
    response.setStatus(501);
    response.setBody(_getErrorPage(501));
    response.addHeader("Content-Type", "text/html");
    return response;
}

Response ResponseHandler::_handleGET(const Request& request, const Server* server, const Location* location) const {
    Response response;
    std::string uri = request.getRequestLine().getUri();
    std::string root = _getServerRoot(server);
    
    if (location) {
        std::string locRoot = _getLocationRoot(location);
        if (!locRoot.empty()) {
            root = locRoot;
        }
    }
    
    std::string filePath = _buildFilePath(uri, root);
    
    // Check if it's a CGI script
    if (_isCGIScript(filePath)) {
        std::string cgiOutput = _executeCGI(filePath, request);
        if (!cgiOutput.empty()) {
            response.setStatus(200);
            response.setBody(cgiOutput);
            response.addHeader("Content-Type", "text/html");
        } else {
            response.setStatus(500);
            response.setBody(_getErrorPage(500));
            response.addHeader("Content-Type", "text/html");
        }
        return response;
    }
    
    // Check if file exists
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        response.setStatus(404);
        response.setBody(_getErrorPage(404));
        response.addHeader("Content-Type", "text/html");
        return response;
    }
    
    // Check if it's a directory
    if (S_ISDIR(fileStat.st_mode)) {
        if (_autoIndex) {
            std::string listing = _generateDirectoryListing(filePath, uri);
            response.setStatus(200);
            response.setBody(listing);
            response.addHeader("Content-Type", "text/html");
        } else {
            // Try to serve index file
            std::string indexPath = filePath + "/index.html";
            if (stat(indexPath.c_str(), &fileStat) == 0) {
                std::string content = loadFile(indexPath);
                response.setStatus(200);
                response.setBody(content);
                response.addHeader("Content-Type", _getMimeType(indexPath));
            } else {
                response.setStatus(403);
                response.setBody(_getErrorPage(403));
                response.addHeader("Content-Type", "text/html");
            }
        }
        return response;
    }
    
    // Serve static file
    std::string content = loadFile(filePath);
    if (!content.empty()) {
        response.setStatus(200);
        response.setBody(content);
        response.addHeader("Content-Type", _getMimeType(filePath));
    } else {
        response.setStatus(500);
        response.setBody(_getErrorPage(500));
        response.addHeader("Content-Type", "text/html");
    }
    
    return response;
}

Response ResponseHandler::_handlePOST(const Request& request, const Server* server, const Location* location) const {
    Response response;
    std::string uri = request.getRequestLine().getUri();
    
    // Handle file upload
    if (uri == "/upload") {
        std::string result = _handleFileUpload(request);
        if (!result.empty()) {
            response.setStatus(201);
            response.setBody(result);
            response.addHeader("Content-Type", "text/html");
        } else {
            response.setStatus(500);
            response.setBody(_getErrorPage(500));
            response.addHeader("Content-Type", "text/html");
        }
        return response;
    }
    
    // Handle CGI POST
    std::string root = _getServerRoot(server);
    if (location) {
        std::string locRoot = _getLocationRoot(location);
        if (!locRoot.empty()) {
            root = locRoot;
        }
    }
    
    std::string filePath = _buildFilePath(uri, root);
    if (_isCGIScript(filePath)) {
        std::string cgiOutput = _executeCGI(filePath, request);
        if (!cgiOutput.empty()) {
            response.setStatus(200);
            response.setBody(cgiOutput);
            response.addHeader("Content-Type", "text/html");
        } else {
            response.setStatus(500);
            response.setBody(_getErrorPage(500));
            response.addHeader("Content-Type", "text/html");
        }
        return response;
    }
    
    response.setStatus(404);
    response.setBody(_getErrorPage(404));
    response.addHeader("Content-Type", "text/html");
    return response;
}

Response ResponseHandler::_handleDELETE(const Request& request, const Server* server, const Location* location) const {
    Response response;
    std::string uri = request.getRequestLine().getUri();
    std::string root = _getServerRoot(server);
    
    if (location) {
        std::string locRoot = _getLocationRoot(location);
        if (!locRoot.empty()) {
            root = locRoot;
        }
    }
    
    std::string filePath = _buildFilePath(uri, root);
    
    if (_deleteFile(filePath)) {
        response.setStatus(204);
        response.setBody("");
    } else {
        response.setStatus(404);
        response.setBody(_getErrorPage(404));
        response.addHeader("Content-Type", "text/html");
    }
    
    return response;
}

std::string ResponseHandler::_getServerRoot(const Server* server) const {
    (void)server; // Suppress unused parameter warning
    // This would need to be implemented based on the actual Server class structure
    // For now, return default root
    return _rootPath;
}

std::string ResponseHandler::_getLocationRoot(const Location* location) const {
    (void)location; // Suppress unused parameter warning
    // This would need to be implemented based on the actual Location class structure
    return "";
}

unsigned int ResponseHandler::_getMaxBodySize(const Server* server) const {
    (void)server; // Suppress unused parameter warning
    // This would need to be implemented based on the actual Server class structure
    return _maxBodySize;
}

bool ResponseHandler::_isAllowedMethod(const std::string& method, const Location* location) const {
    (void)method; // Suppress unused parameter warning
    (void)location; // Suppress unused parameter warning
    // This would need to be implemented based on the actual Location class structure
    return true;
}

std::string ResponseHandler::_buildFilePath(const std::string& uri, const std::string& root) const {
    std::string path = root;
    if (path[path.length() - 1] != '/') {
        path += "/";
    }
    
    // Remove leading slash from URI
    std::string cleanUri = uri;
    if (cleanUri[0] == '/') {
        cleanUri = cleanUri.substr(1);
    }
    
    path += cleanUri;
    return path;
}

std::string ResponseHandler::_getErrorPage(int statusCode) const {
    std::map<int, std::string>::const_iterator it = _errorPages.find(statusCode);
    if (it != _errorPages.end()) {
        return loadFile(it->second);
    }
    
    // Default error pages
    switch (statusCode) {
        case 404:
            return "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        case 405:
            return "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed.</p></body></html>";
        case 413:
            return "<html><head><title>413 Request Entity Too Large</title></head><body><h1>413 Request Entity Too Large</h1><p>The request entity is too large.</p></body></html>";
        case 500:
            return "<html><head><title>500 Internal Server Error</title></head><body><h1>500 Internal Server Error</h1><p>An internal server error occurred.</p></body></html>";
        case 501:
            return "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1><p>The requested method is not implemented.</p></body></html>";
        default:
            return "<html><head><title>Error</title></head><body><h1>Error</h1><p>An error occurred.</p></body></html>";
    }
}

std::string ResponseHandler::_executeCGI(const std::string& scriptPath, const Request& request) const {
    // Create temporary file for CGI output
    std::string tempFile = "/tmp/cgi_output_" + toString(getpid()) + "_" + timeToString(time(NULL));
    
    // Set up environment variables
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = request.getRequestLine().getMethod();
    env["REQUEST_URI"] = request.getRequestLine().getUri();
    env["QUERY_STRING"] = ""; // Would need to extract from URI
    env["SERVER_PROTOCOL"] = request.getRequestLine().getVersion();
    env["HTTP_HOST"] = request.getRequestHeaders().getHeaderValue("host");
    env["HTTP_USER_AGENT"] = request.getRequestHeaders().getHeaderValue("user-agent");
    env["HTTP_ACCEPT"] = request.getRequestHeaders().getHeaderValue("accept");
    env["CONTENT_TYPE"] = request.getRequestHeaders().getHeaderValue("content-type");
    env["CONTENT_LENGTH"] = toString(request.getRequestBody().getContentLength());
    
    // Create environment array
    std::vector<std::string> envArray;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
        envArray.push_back(it->first + "=" + it->second);
    }
    
    // Create argv array
    std::vector<std::string> argv;
    argv.push_back("/usr/bin/python3");
    argv.push_back(scriptPath);
    
    // Convert to char* arrays
    std::vector<char*> envPtrs;
    for (size_t i = 0; i < envArray.size(); ++i) {
        envPtrs.push_back(const_cast<char*>(envArray[i].c_str()));
    }
    envPtrs.push_back(NULL);
    
    std::vector<char*> argvPtrs;
    for (size_t i = 0; i < argv.size(); ++i) {
        argvPtrs.push_back(const_cast<char*>(argv[i].c_str()));
    }
    argvPtrs.push_back(NULL);
    
    // Fork and execute
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        int outputFd = open(tempFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outputFd != -1) {
            dup2(outputFd, STDOUT_FILENO);
            close(outputFd);
        }
        
        // Redirect stdin if there's POST data
        if (request.getRequestLine().getMethod() == "POST" && !request.getRequestBody().getRawData().empty()) {
            int inputFd = open("/tmp/cgi_input", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (inputFd != -1) {
                write(inputFd, request.getRequestBody().getRawData().c_str(), request.getRequestBody().getRawData().length());
                close(inputFd);
                
                inputFd = open("/tmp/cgi_input", O_RDONLY);
                if (inputFd != -1) {
                    dup2(inputFd, STDIN_FILENO);
                    close(inputFd);
                }
            }
        }
        
        execve(argvPtrs[0], &argvPtrs[0], &envPtrs[0]);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        // Read output
        std::string output = loadFile(tempFile);
        unlink(tempFile.c_str());
        unlink("/tmp/cgi_input");
        
        return output;
    }
    
    return "";
}

std::string ResponseHandler::_handleFileUpload(const Request& request) const {
    std::string contentType = request.getRequestHeaders().getHeaderValue("content-type");
    std::string body = request.getRequestBody().getRawData();
    
    if (contentType.find("multipart/form-data") != std::string::npos) {
        // Parse multipart data
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            std::string boundary = "--" + contentType.substr(boundaryPos + 9);
            size_t pos = body.find(boundary);
            if (pos != std::string::npos) {
                pos = body.find("\r\n\r\n", pos);
                if (pos != std::string::npos) {
                    pos += 4;
                    size_t endPos = body.find(boundary, pos);
                    if (endPos != std::string::npos) {
                        std::string fileData = body.substr(pos, endPos - pos - 2);
                        
                        // Generate unique filename
                        std::string filename = "upload_" + toString(time(NULL)) + ".txt";
                        std::string filepath = _uploadPath + "/" + filename;
                        
                        // Write file
                        std::ofstream file(filepath.c_str());
                        if (file.is_open()) {
                            file.write(fileData.c_str(), fileData.length());
                            file.close();
                            
                            return "<html><head><title>File Uploaded</title></head><body><h1>File Uploaded Successfully</h1><p>File saved as: " + filename + "</p></body></html>";
                        }
                    }
                }
            }
        }
    }
    
    return "";
}

bool ResponseHandler::_deleteFile(const std::string& filePath) const {
    return (unlink(filePath.c_str()) == 0);
}

std::string ResponseHandler::_generateDirectoryListing(const std::string& path, const std::string& uri) const {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return "";
    }
    
    std::string html = "<html><head><title>Index of " + uri + "</title></head><body>";
    html += "<h1>Index of " + uri + "</h1><hr><ul>";
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        
        std::string fullPath = path + "/" + name;
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            std::string link = uri;
            if (link[link.length() - 1] != '/') {
                link += "/";
            }
            link += name;
            
            if (S_ISDIR(fileStat.st_mode)) {
                html += "<li><a href=\"" + link + "/\">" + name + "/</a></li>";
            } else {
                html += "<li><a href=\"" + link + "\">" + name + "</a></li>";
            }
        }
    }
    
    html += "</ul><hr></body></html>";
    closedir(dir);
    return html;
}

std::string ResponseHandler::_getMimeType(const std::string& path) const {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        std::map<std::string, std::string>::const_iterator it = _mimeTypes.find(ext);
        if (it != _mimeTypes.end()) {
            return it->second;
        }
    }
    return "text/plain";
}

bool ResponseHandler::_isCGIScript(const std::string& path) const {
    return (path.find(".py") != std::string::npos && path.find(_cgiPath) != std::string::npos);
}

std::string ResponseHandler::_urlDecode(const std::string& str) const {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            iss >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string ResponseHandler::_urlEncode(const std::string& str) const {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~') {
            result += str[i];
        } else {
            char hex[4];
            snprintf(hex, sizeof(hex), "%%%02X", static_cast<unsigned char>(str[i]));
            result += hex;
        }
    }
    return result;
}

void ResponseHandler::setErrorPages(const std::map<int, std::string>& errorPages) {
    _errorPages = errorPages;
}

void ResponseHandler::setAutoIndex(bool autoIndex) {
    _autoIndex = autoIndex;
} 