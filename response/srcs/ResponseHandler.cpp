#include "../include/ResponseHandler.hpp"
#include "../include/ErrorResponse.hpp"
#include "../include/ReturnHandler.hpp"
#include "../include/FileResponse.hpp"
#include "../include/CgiHandler.hpp"
#include "../../Connection.hpp"
#include "../../conf/Index.hpp"
#include "../../conf/ErrorPage.hpp"
#include "../../conf/AutoIndex.hpp"
#include "../../conf/Root.hpp"
#include "../../conf/Location.hpp"
#include "../../conf/Server.hpp"
#include "../../conf/IDirective.hpp"
#include <cstddef>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sstream>


static std::string _normalizeUri(const std::string& uri) {
    std::string result;
    bool prevSlash = false;
    
    for (size_t i = 0; i < uri.size(); ++i) {
        if (uri[i] == '/') {
            if (!prevSlash) {
                result += '/';
            }
            prevSlash = true;
        } else {
            result += uri[i];
            prevSlash = false;
        }
    }
    return result.empty() ? "/" : result;
}

std::string ResponseHandler::_getRootPath(Connection* conn) {
    if (!conn) return "www";
    Root* root = conn->getRoot();
    if (root && root->getPath()) return std::string(root->getPath());
    return "www";
}
std::vector<std::string> ResponseHandler::_getIndexFiles(Connection* conn) {
    std::vector<std::string> indexFiles;
    if (!conn) {
        indexFiles.push_back("index.html");
        indexFiles.push_back("index.htm");
        return indexFiles;
    }
    Index* index = conn->getIndex();
    if (index) {
        char** files = index->getFiles();
        if (files) {
            for (int i = 0; files[i] != NULL; ++i) {
                indexFiles.push_back(std::string(files[i]));
            }
        }
    }
    if (indexFiles.empty()) {
        indexFiles.push_back("index.html");
        indexFiles.push_back("index.htm");
    }
    return indexFiles;
}
bool ResponseHandler::_getAutoIndex(Connection* conn) {
    if (!conn) return false;
    AutoIndex* autoindex = conn->getAutoIndex();
    if (autoindex) return autoindex->getState();
    return false;
}
std::map<int, std::string> ResponseHandler::_getErrorPages(Connection* conn) {
    std::map<int, std::string> errorPages;
    if (!conn) return errorPages;
    const Location* location = conn->getLocation();
    if (location) {
        for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); dit != location->directives.end(); ++dit) {
            if ((*dit)->getType() == ERROR_PAGE) {
                ErrorPage* ep = static_cast<ErrorPage*>(*dit);
                if (ep && ep->getUri() && ep->getUri()[0])
                    errorPages[ep->getCode()] = std::string(ep->getUri());
            }
        }
    }
    if (conn->conServer) {
        for (std::vector<IDirective*>::const_iterator dit = conn->conServer->directives.begin(); dit != conn->conServer->directives.end(); ++dit) {
            if ((*dit)->getType() == ERROR_PAGE) {
                ErrorPage* ep = static_cast<ErrorPage*>(*dit);
                if (ep && ep->getUri() && ep->getUri()[0] && errorPages.find(ep->getCode()) == errorPages.end())
                    errorPages[ep->getCode()] = std::string(ep->getUri());
            }
        }
    }
    return errorPages;
}


std::string ResponseHandler::_buildFilePath(const std::string& uri, const std::string& root, const Location* location) {
    std::string path = root;
    while (!path.empty() && path[path.length() - 1] == '/') path.erase(path.length() - 1);

    std::string cleanUri = uri;
    if (location && location->getUri()) {
        std::string locUri = std::string(location->getUri());
        
        if (!locUri.empty() && locUri[0] == '.') {
            while (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
            if (!cleanUri.empty()) path += "/" + cleanUri;

            return path;
        }
        
        if (!locUri.empty() && locUri[0] == '/') locUri = locUri.substr(1);
        std::string prefix = "/" + locUri;
        if (cleanUri.find(prefix) == 0) {
            cleanUri = cleanUri.substr(prefix.length());
            if (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
        }
    }
    while (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
    if (!cleanUri.empty()) path += "/" + cleanUri;
    return path;
}

std::string ResponseHandler::_getMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        if (ext == ".html") return "text/html";
        if (ext == ".htm") return "text/html";
        if (ext == ".css") return "text/css";
        if (ext == ".js") return "application/javascript";
        if (ext == ".json") return "application/json";
        if (ext == ".png") return "image/png";
        if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
        if (ext == ".gif") return "image/gif";
        if (ext == ".svg") return "image/svg+xml";
        if (ext == ".ico") return "image/x-icon";
        if (ext == ".txt") return "text/plain";
        if (ext == ".pdf") return "application/pdf";
        if (ext == ".zip") return "application/zip";
        if (ext == ".tar") return "application/x-tar";
        if (ext == ".gz") return "application/gzip";
        if (ext == ".mp3") return "audio/mpeg";
        if (ext == ".mp4") return "video/mp4";
        if (ext == ".xml") return "application/xml";
        if (ext == ".htm") return "text/html";
        if (ext == ".shtml") return "text/html";
        if (ext == ".css") return "text/css";
        if (ext == ".xml") return "text/xml";
        if (ext == ".gif") return "image/gif";
        if (ext == ".jpeg") return "image/jpeg";
        if (ext == ".jpg") return "image/jpeg";
        if (ext == ".js") return "application/javascript";
        if (ext == ".atom") return "application/atom+xml";
        if (ext == ".rss") return "application/rss+xml";
        if (ext == ".mml") return "text/mathml";
        if (ext == ".txt") return "text/plain";
        if (ext == ".jad") return "text/vnd.sun.j2me.app-descriptor";
        if (ext == ".wml") return "text/vnd.wap.wml";
        if (ext == ".htc") return "text/x-component";
        if (ext == ".avif") return "image/avif";
        if (ext == ".png") return "image/png";
        if (ext == ".svg") return "image/svg+xml";
        if (ext == ".svgz") return "image/svg+xml";
        if (ext == ".tif") return "image/tiff";
        if (ext == ".tiff") return "image/tiff";
        if (ext == ".wbmp") return "image/vnd.wap.wbmp";
        if (ext == ".webp") return "image/webp";
        if (ext == ".ico") return "image/x-icon";
        if (ext == ".jng") return "image/x-jng";
        if (ext == ".bmp") return "image/x-ms-bmp";
        if (ext == ".woff") return "font/woff";
        if (ext == ".woff2") return "font/woff2";
        if (ext == ".jar") return "application/java-archive";
        if (ext == ".war") return "application/java-archive";
        if (ext == ".ear") return "application/java-archive";
        if (ext == ".json") return "application/json";
        if (ext == ".hqx") return "application/mac-binhex40";
        if (ext == ".doc") return "application/msword";
        if (ext == ".pdf") return "application/pdf";
        if (ext == ".ps") return "application/postscript";
        if (ext == ".eps") return "application/postscript";
        if (ext == ".ai") return "application/postscript";
        if (ext == ".rtf") return "application/rtf";
        if (ext == ".m3u8") return "application/vnd.apple.mpegurl";
        if (ext == ".kml") return "application/vnd.google-earth.kml+xml";
        if (ext == ".kmz") return "application/vnd.google-earth.kmz";
        if (ext == ".xls") return "application/vnd.ms-excel";
        if (ext == ".eot") return "application/vnd.ms-fontobject";
        if (ext == ".ppt") return "application/vnd.ms-powerpoint";
        if (ext == ".odg") return "application/vnd.oasis.opendocument.graphics";
        if (ext == ".odp") return "application/vnd.oasis.opendocument.presentation";
        if (ext == ".ods") return "application/vnd.oasis.opendocument.spreadsheet";
        if (ext == ".odt") return "application/vnd.oasis.opendocument.text";
        if (ext == ".wmlc") return "application/vnd.wap.wmlc";
        if (ext == ".wasm") return "application/wasm";
        if (ext == ".7z") return "application/x-7z-compressed";
        if (ext == ".cco") return "application/x-cocoa";
        if (ext == ".jardiff") return "application/x-java-archive-diff";
        if (ext == ".jnlp") return "application/x-java-jnlp-file";
        if (ext == ".run") return "application/x-makeself";
        if (ext == ".pl") return "application/x-perl";
        if (ext == ".pm") return "application/x-perl";
        if (ext == ".prc") return "application/x-pilot";
        if (ext == ".pdb") return "application/x-pilot";
        if (ext == ".rar") return "application/x-rar-compressed";
        if (ext == ".rpm") return "application/x-redhat-package-manager";
        if (ext == ".sea") return "application/x-sea";
        if (ext == ".swf") return "application/x-shockwave-flash";
        if (ext == ".sit") return "application/x-stuffit";
        if (ext == ".tcl") return "application/x-tcl";
        if (ext == ".tk") return "application/x-tcl";
        if (ext == ".der") return "application/x-x509-ca-cert";
        if (ext == ".pem") return "application/x-x509-ca-cert";
        if (ext == ".crt") return "application/x-x509-ca-cert";
        if (ext == ".xpi") return "application/x-xpinstall";
        if (ext == ".xhtml") return "application/xhtml+xml";
        if (ext == ".xspf") return "application/xspf+xml";
        if (ext == ".zip") return "application/zip";
        if (ext == ".bin") return "application/octet-stream";
        if (ext == ".exe") return "application/octet-stream";
        if (ext == ".dll") return "application/octet-stream";
        if (ext == ".deb") return "application/octet-stream";
        if (ext == ".dmg") return "application/octet-stream";
        if (ext == ".iso") return "application/octet-stream";
        if (ext == ".img") return "application/octet-stream";
        if (ext == ".msi") return "application/octet-stream";
        if (ext == ".msp") return "application/octet-stream";
        if (ext == ".msm") return "application/octet-stream";
        if (ext == ".mid") return "audio/midi";
        if (ext == ".midi") return "audio/midi";
        if (ext == ".kar") return "audio/midi";
        if (ext == ".mp3") return "audio/mpeg";
        if (ext == ".ogg") return "audio/ogg";
        if (ext == ".m4a") return "audio/x-m4a";
        if (ext == ".ra") return "audio/x-realaudio";
        if (ext == ".3gpp") return "video/3gpp";
        if (ext == ".3gp") return "video/3gpp";
        if (ext == ".ts") return "video/mp2t";
        if (ext == ".mp4") return "video/mp4";
        if (ext == ".mpeg") return "video/mpeg";
        if (ext == ".mpg") return "video/mpeg";
        if (ext == ".mov") return "video/quicktime";
        if (ext == ".webm") return "video/webm";
        if (ext == ".flv") return "video/x-flv";
        if (ext == ".m4v") return "video/x-m4v";
        if (ext == ".mng") return "video/x-mng";
        if (ext == ".asx") return "video/x-ms-asf";
        if (ext == ".asf") return "video/x-ms-asf";
        if (ext == ".wmv") return "video/x-ms-wmv";
        if (ext == ".avi") return "video/x-msvideo";
        if (ext == ".py") return "text/x-python";
        if (ext == ".sh") return "text/x-shellscript";
        if (ext == ".php") return "text/x-php";
    }
    return "application/octet-stream";
}

// TODO: LOCATION FILE SHOULD BE ENCODED. "FILE%201.TXT" => "FILE 1.TXT"

std::string ResponseHandler::_generateDirectoryListing(const std::string& path, const std::string& uri) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return "";
    std::string html = "<html><head><title>Index of " + uri + "</title></head><body><h1>Index of " + uri + "</h1><hr><ul>";
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string fullPath = path + "/" + name;
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            std::string link = uri;
            if (link[link.length() - 1] != '/') link += "/";
            link += name;
            if (S_ISDIR(fileStat.st_mode)) html += "<li><a href='" + link + "/'>" + name + "/</a></li>";
            else html += "<li><a href='" + link + "'>" + name + "</a></li>";
        }
    }
    html += "</ul><hr></body></html>";
    closedir(dir);
    return html;
}

Response ResponseHandler::handleRequest(Connection* conn) 
{
    // std::cout << RED  << "in handle request "<< RESET << std::endl;
    const Request& request = *conn->req;
    if (conn->req->getStatusCode() != OK)
    {
        return ErrorResponse::createErrorResponseWithMapping(conn,conn->req->getStatusCode() );
    }
    std::string method = request.getRequestLine().getMethod();
    const Location* location = conn->getLocation();
    Return* ret = conn->getReturnDirective();
    if (ret) return ReturnHandler::handle(conn);
    std::string root;
    Root* locRoot = NULL;
    if (location) {
        for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); dit != location->directives.end(); ++dit) {
            if ((*dit)->getType() == ROOT) {
                locRoot = static_cast<Root*>(*dit);
                break;
            }
        }
    }
    if (locRoot && locRoot->getPath())
        root = std::string(locRoot->getPath());
    else
        root = _getRootPath(conn);
    // std::cout << CYAN << "root " << root <<  RESET << std::endl;
    std::string uri = _normalizeUri(request.getRequestLine().getUri());
    std::string filePath = _buildFilePath(uri, root, location);
    // std::cout << CYAN << "filepath " <<  filePath <<  RESET << std::endl;
    
    if (location && location->getUri() && location->getUri()[0] == '.') {        
        if (conn->cgiExecuted == false) {
            std::cout << YELLOW << "Executing CGI..." << RESET << std::endl;
            conn->isCgi = true;
            conn->cgiStartTime = time(NULL);
            return CgiHandler::executeCgi(conn, filePath);
        }
    }
    
    if (method == "GET") {
        struct stat fileStat;
        bool isDir = false;
        if (stat(filePath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) isDir = true;
        std::vector<std::string> indexFiles = _getIndexFiles(conn);
        if (isDir) {
            for (size_t i = 0; i < indexFiles.size(); ++i) {
                std::string indexPath = filePath;
                if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
                    indexPath += "/";
                indexPath += indexFiles[i];
                std::cout << CYAN << "indexPaht " <<  indexPath <<  RESET << std::endl;
                if (stat(indexPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                    return FileResponse::serve(indexPath, _getMimeType(indexPath), 200);
                }
            }
        }
        if (isDir) {
            if (_getAutoIndex(conn)) {
                std::string listing = _generateDirectoryListing(filePath, uri);
                Response response(200);
                response.addHeader("Content-Type", "text/html");
                std::ostringstream oss;
                oss << listing.length();
                response.addHeader("Content-Length", oss.str());
                response.addHeader("Connection", "close");
                std::string responseStr = response.build();
                responseStr += listing;
                send(conn->fd, responseStr.c_str(), responseStr.size(), MSG_NOSIGNAL);//TODO
                conn->fileSendState = 3;
                return response;
            }
            return ErrorResponse::createForbiddenResponse(conn);
        }
        std::cout << RED << filePath << RESET << std::endl;
        if (stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            return FileResponse::serve(filePath, _getMimeType(filePath), 200);
        }
        std::map<int, std::string> errorPages = _getErrorPages(conn);
        int errCode = 404;
        std::string errPage = errorPages[errCode];
        if (!errPage.empty()) {
            if (stat(errPage.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                return FileResponse::serve(errPage, _getMimeType(errPage), errCode);
            }
        }
        return ErrorResponse::createNotFoundResponse(conn);
    }
    else if (method == "POST")
    {
        struct stat fileStat;
        char fPath[] = "www/201.html";
        if (stat(fPath , &fileStat) != 0) {
            return ErrorResponse::createNotFoundResponse(conn);
        }
        Response response(201);
        std::string lastPath = request.getRequestBody().getTempFile().path();
        response.addHeader("location", lastPath);
        response.setFilePath(fPath);
        response.setContentType(_getMimeType(fPath));
        response.setFileSize(static_cast<size_t>(fileStat.st_size));
        response.addHeader("Connection", "close");
        return response;
    }
    else if (method == "DELETE")
    {
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            return ErrorResponse::createNotFoundResponse(conn);
        }
        if (S_ISDIR(fileStat.st_mode)) {
            return ErrorResponse::createForbiddenResponse(conn);
        }
        if (unlink(filePath.c_str()) != 0) {
            return ErrorResponse::createInternalErrorResponse(conn);
        }
        Response response(204);
        response.addHeader("Content-Length", "0");
        return response;
    }
    return ErrorResponse::createInternalErrorResponse(conn);
}
