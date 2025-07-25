#include "../include/ResponseHandler.hpp"
#include "../include/ErrorResponse.hpp"
#include "../include/DirectoryListing.hpp"
#include "../include/MimeTypes.hpp"
#include "../include/ReturnHandler.hpp"
#include "../include/FileResponse.hpp"
#include "../../Connection.hpp"
#include "../../conf/Index.hpp"
#include "../../conf/LimitExcept.hpp"
#include "../../conf/ErrorPage.hpp"
#include "../../conf/AutoIndex.hpp"
#include "../../conf/Root.hpp"
#include "../../conf/Location.hpp"
#include "../../conf/Server.hpp"
#include "../../conf/IDirective.hpp"
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

static std::string _normalizeUri(const std::string& uri) {
    std::string result;
    bool prevSlash = false;
    for (size_t i = 0; i < uri.size(); ++i) {
        if (uri[i] == '/') {
            if (!prevSlash) result += '/';
            prevSlash = true;
        } else {
            result += uri[i];
            prevSlash = false;
        }
    }
    if (result.size() > 1 && result[result.size() - 1] == '/')
        result.erase(result.size() - 1);
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
    }
    return "application/octet-stream";
}
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
    const Request& request = *conn->req;
    if (conn->req->getStatusCode() != OK)
    {
        return ErrorResponse::createErrorResponseWithMapping(conn,conn->req->getStatusCode() );
    }
    std::string method = request.getRequestLine().getMethod();
    std::vector<std::string> allowed = conn->_getAllowedMethods();
    for (std::vector<std::string>::const_iterator it = allowed.begin(); it != allowed.end(); it++)
        std::cout <<BGREEN << (*it) << RESET << std::endl;

    if (!conn->_isAllowedMethod(method, allowed))
        return ErrorResponse::createMethodNotAllowedResponse(conn ,allowed);
    const Location* location = conn->getLocation();
    // if LOCATION is CGI set 
    // if (location) {
    //     std::cout << YELLOW <<  location->getUri()  <<  RESET << std::endl;
    //     Root* locRoot = NULL;
    //     for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); dit != location->directives.end(); ++dit) {
    //         if ((*dit)->getType() == ROOT) {
    //             locRoot = static_cast<Root*>(*dit);
    //             break;
    //         }
    //     }
    //     std::cout << YELLOW <<  locRoot->getPath()  <<  RESET << std::endl;
    // }

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
    std::cout << CYAN << "root " << root <<  RESET << std::endl;
    std::string uri = _normalizeUri(request.getRequestLine().getUri());
    std::string filePath = _buildFilePath(uri, root, location);
    std::cout << CYAN << "filepath " <<  filePath <<  RESET << std::endl;
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
                std::string dir = root.empty() ? "www" : root;
                std::stringstream ss;
                ss << dir << "/autoindex_" << getpid() << ".html";
                std::string tmpFile = ss.str();
                std::ofstream out(tmpFile.c_str());
                out << listing;
                out.close();
                struct stat tmpStat;
                if (stat(tmpFile.c_str(), &tmpStat) == 0 && S_ISREG(tmpStat.st_mode)) {
                    return FileResponse::serve(tmpFile, "text/html", 200);
                }
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
        response.addHeader("location", "www/201.html");
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
