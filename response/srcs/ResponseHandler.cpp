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


/* std::string ResponseHandler::_buildFilePath(const std::string& uri, const std::string& root, const Location* location) {
    std::string path = root;
    // << BGREEN << std::endl;
    // << "root start -> " << path << std::endl;
    //example www/////// ---> www
    while (!path.empty() && path[path.length() - 1] == '/')
    {
        // << "root loop ->" << path << std::endl;
        path.erase(path.length() - 1);
    }
    // << "root ends ->" << path << std::endl;

    std::string cleanUri = uri;
    if (location && location->getUri()) {
        std::string locUri = std::string(location->getUri());
        
        if (!locUri.empty() && locUri[0] == '.') {
            while (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
            if (!cleanUri.empty()) path += "/" + cleanUri;

            return path;
        }
        
        if (!locUri.empty() && locUri[0] == '/') locUri = locUri.substr(1);
        
        std::string locationPrefix = "/" + locUri;

    //     std::string prefix = "/" + locUri;
    //     // << "clean Uri --> " <<  cleanUri << std::endl;
    //     // << "prefix --> " <<  prefix << std::endl;
    //     if (cleanUri.find(prefix) == 0) {
    //         cleanUri = cleanUri.substr(prefix.length());
    //         if (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
    //     }
    // }
    // while (!cleanUri.empty() && cleanUri[0] == '/') cleanUri = cleanUri.substr(1);
    // if (!cleanUri.empty()) path += "/" + cleanUri;

        if (cleanUri.length() > locationPrefix.length() && 
                   cleanUri.substr(0, locationPrefix.length()) == locationPrefix) {
            
            char nextChar = cleanUri[locationPrefix.length()];
            if (nextChar == '/') {

                std::string remainder = cleanUri.substr(locationPrefix.length());
                
                while (!remainder.empty() && remainder[0] == '/') {
                    remainder = remainder.substr(1);
                }
                
                if (!remainder.empty()) {
                    path += "/" + remainder;
                }
                return path;
            }
            // If nextChar is not '/', it's a different file (like /kapouet.html)
            // Fall through to default handling
        }
    }
    //default handle
    while (!cleanUri.empty() && cleanUri[0] == '/') {
        cleanUri = cleanUri.substr(1);
    }
    
    if (!cleanUri.empty()) {
        path += "/" + cleanUri;
    }
    
    // << RESET << std::endl;

    return path;
}
 */

// std::string ResponseHandler::_buildFilePath(const std::string& uri, const std::string& root, const Location* location) {
//     std::string path = root;
    
//     // Remove trailing slashes from root
//     while (!path.empty() && path[path.length() - 1] == '/') {
//         path.erase(path.length() - 1);
//     }

//     std::string cleanUri = uri;
    
//     if (location && location->getUri()) {
//         std::string locUri = std::string(location->getUri());
        
//         if (!locUri.empty() && locUri[0] == '.') {
//             // For CGI, just append the full URI (don't strip anything)
//             while (!cleanUri.empty() && cleanUri[0] == '/') {
//                 cleanUri = cleanUri.substr(1);
//             }
//             if (!cleanUri.empty()) {
//                 path += "/" + cleanUri;
//             }
//             return path;
//         }
//     }
    
//     while (!cleanUri.empty() && cleanUri[0] == '/') {
//         cleanUri = cleanUri.substr(1);
//     }
    
//     if (!cleanUri.empty()) {
//         path += "/" + cleanUri;
//     }
    
//     return path;
// }


std::string ResponseHandler::_buildFilePath(const std::string& uri, const std::string& root, const Location* location) {
std::string path = root;
    
    // Remove trailing slashes from root (e.g., "www/////" -> "www")
    while (!path.empty() && path[path.length() - 1] == '/') {
        path.erase(path.length() - 1);
    }
    
    std::string cleanUri = uri;
    
    // Handle location-specific path mapping
    if (location && location->getUri()) {
        std::string locUri = std::string(location->getUri());
        
        // Special case for CGI locations (starting with '.')
        if (!locUri.empty() && locUri[0] == '.') {
            // For CGI, don't strip location prefix, just clean leading slashes
            while (!cleanUri.empty() && cleanUri[0] == '/') {
                cleanUri = cleanUri.substr(1);
            }
            if (!cleanUri.empty()) {
                path += "/" + cleanUri;
            }
            return path;
        }
        
        // Normal location handling - strip location prefix from URI
        // Example: location /kapouet with root /tmp/www
        // URI: /kapouet/pouic/toto/pouet -> /tmp/www/pouic/toto/pouet
        
        std::string locationPrefix = locUri;
        
        // Normalize location prefix (ensure it starts with /)
        if (!locationPrefix.empty() && locationPrefix[0] != '/') {
            locationPrefix = "/" + locationPrefix;
        }
        
        // Remove trailing slash from location prefix for comparison
        while (!locationPrefix.empty() && locationPrefix[locationPrefix.length() - 1] == '/') {
            locationPrefix.erase(locationPrefix.length() - 1);
        }
        
        // << BBLUE << "Location prefix: '" << locationPrefix << "'" << RESET << std::endl;
        // << BBLUE << "Original URI: '" << cleanUri << "'" << RESET << std::endl;
        
        // Check if URI starts with location prefix
        if (cleanUri.length() >= locationPrefix.length() && 
            cleanUri.substr(0, locationPrefix.length()) == locationPrefix) {
            
            // Check what comes after the prefix
            if (cleanUri.length() == locationPrefix.length()) {
                // Exact match (e.g., URI="/kapouet", location="/kapouet")
                // Result should be just the root path
                // << BBLUE << "Exact location match, using root: '" << path << "'" << RESET << std::endl;
                return path;
            }
            
            char nextChar = cleanUri[locationPrefix.length()];
            if (nextChar == '/') {
                // Valid prefix match followed by '/'
                // Strip the location prefix and the following '/'
                std::string remainder = cleanUri.substr(locationPrefix.length() + 1);
                
                // << BBLUE << "Stripped remainder: '" << remainder << "'" << RESET << std::endl;
                
                // Clean any additional leading slashes
                while (!remainder.empty() && remainder[0] == '/') {
                    remainder = remainder.substr(1);
                }
                
                // Append remainder to root path
                if (!remainder.empty()) {
                    path += "/" + remainder;
                }
                
                // << BBLUE << "Final path after location processing: '" << path << "'" << RESET << std::endl;
                return path;
            }
            // If nextChar is not '/', it might be a different resource
            // (like /kapouet.html when location is /kapouet)
            // Fall through to default handling
        }
    }
    
    // Default handling - no location stripping, just append URI to root
    // Remove leading slashes from URI
    while (!cleanUri.empty() && cleanUri[0] == '/') {
        cleanUri = cleanUri.substr(1);
    }
    
    if (!cleanUri.empty()) {
        path += "/" + cleanUri;
    }
    
    // << BBLUE << "Default path handling result: '" << path << "'" << RESET << std::endl;
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

std::string ResponseHandler::_getRootPath(Connection* conn) {
    if (!conn) return "www";
    Root* root = conn->getRoot();
    if (root && root->getPath()) return std::string(root->getPath());
    return "www";
}

Response ResponseHandler::handleRequest(Connection* conn) 
{
    // << RED  << "in handle request "<< RESET << std::endl;
    const Request& request = *conn->req;
    if (conn->req->getStatusCode() != OK)
    {
        std::cout << conn->req->getStatusCode() << std::endl;
        
        return ErrorResponse::createErrorResponseWithMapping(conn,conn->req->getStatusCode() );
    }
    std::string method = request.getRequestLine().getMethod();
    // << method << std::endl;

    std::vector<std::string> allowed = conn->_getAllowedMethods();
    // for (std::vector<std::string>::const_iterator it = allowed.begin(); it != allowed.end(); it++)
        // <<BGREEN << (*it) << RESET << std::endl;

    if (!conn->_isAllowedMethod(method, allowed))
        return ErrorResponse::createMethodNotAllowedResponse(conn ,allowed);
    const Location* location = conn->getLocation();
    // if (location)
        // << BG_GREEN << "location uri " << location->getUri() << RESET << std::endl;
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
    // << CYAN << "methods " << method <<  RESET << std::endl;
    // << CYAN << "root " << root <<  RESET << std::endl;
    std::string uri = conn->_normalizeUri(request.getRequestLine().getUri());
    // << CYAN << "After nomalize uri " << uri << RESET << std::endl;
    std::string filePath = _buildFilePath(uri, root, location);
    // << CYAN << "filepath " <<  filePath <<  RESET << std::endl;
    
    struct stat fileStat;
    bool pathExists = (stat(filePath.c_str(), &fileStat) == 0);
    bool isDir = pathExists && S_ISDIR(fileStat.st_mode);
    bool isFile = pathExists && S_ISREG(fileStat.st_mode);
    
    // should the uri bien a file
    if (location && location->getUri() && location->getUri()[0] == '.' && !isDir) {   
          
        if (conn->cgiExecuted == false) {
            // << YELLOW << "Executing CGI..." << RESET << std::endl;
            conn->isCgi = true;
            conn->cgiStartTime = time(NULL);
            return CgiHandler::executeCgi(conn, filePath);
        }
    }
    
    if (method == "GET") {
        // << CYAN << "GET request for filepath: " << filePath << RESET << std::endl;
        
        
        // << CYAN << "Path exists: " << (pathExists ? "YES" : "NO") 
                // << ", Is dir: " << (isDir ? "YES" : "NO") 
                // << ", Is file: " << (isFile ? "YES" : "NO") << RESET << std::endl;
        
        // Case 1: Direct file request
        if (isFile) {
            // << BGREEN << "Serving file directly: " << filePath << RESET << std::endl;
            return FileResponse::serve(filePath, _getMimeType(filePath), 200);
        }
        
        // Case 2: Directory or location root - look for index files
        if (isDir) {
            std::vector<std::string> indexFiles = _getIndexFiles(conn);
            // << BYELLOW << "Looking for index files..." << RESET << std::endl;
            
            std::string searchDir = filePath;
            
            // If filePath doesn't exist but we have a location, 
            // it might be an exact location match like "/form"
            if (!pathExists && location) {
                // Try looking in a subdirectory with location name
                std::string locationName = location->getUri();
                if (!locationName.empty() && locationName[0] == '/') {
                    locationName = locationName.substr(1);
                }
                
                // Try: root + location_name (e.g., "www" + "form" = "www/form")
                std::string altPath = root;
                if (!altPath.empty() && altPath[altPath.length() - 1] != '/') {
                    altPath += "/";
                }
                altPath += locationName;
                
                // << BYELLOW << "Trying alternative path: " << altPath << RESET << std::endl;
                
                if (stat(altPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
                    searchDir = altPath;
                    isDir = true;
                }
            }
            
            if (isDir || stat(searchDir.c_str(), &fileStat) == 0) {
                for (size_t i = 0; i < indexFiles.size(); ++i) {
                    std::string indexPath = searchDir;
                    if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/') {
                        indexPath += "/";
                    }
                    indexPath += indexFiles[i];
                    
                    // << BYELLOW << "Trying index file: " << indexPath << RESET << std::endl;
                    
                    if (stat(indexPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                        // << BGREEN << "Found index file: " << indexPath << RESET << std::endl;
                        return FileResponse::serve(indexPath, _getMimeType(indexPath), 200);
                    }
                }
                
                // No index file found, check if directory listing is enabled
                if (_getAutoIndex(conn)) {
                    std::string listing = _generateDirectoryListing(searchDir, uri);
                    Response response(200);
                    response.addHeader("Content-Type", "text/html");
                    std::ostringstream oss;
                    oss << listing.length();
                    response.addHeader("Content-Length", oss.str());
                    response.addHeader("Connection", "close");
                    std::string responseStr = response.build();
                    responseStr += listing;
                    send(conn->fd, responseStr.c_str(), responseStr.size(), MSG_NOSIGNAL);
                    conn->fileSendState = 3;
                    return response;
                }
                
                // Directory exists but no index file and no autoindex
                return ErrorResponse::createForbiddenResponse(conn);
            }
        }
        
        // Case 3: File/directory not found
        // << BRED << "Path not found: " << filePath << RESET << std::endl;
        return ErrorResponse::createNotFoundResponse(conn);
    }
    else if (method == "POST")
    {
        struct stat fileStat;
        if (stat(filePath.c_str() , &fileStat) != 0) {
            return ErrorResponse::createNotFoundResponse(conn);
        }
        Response response(201);
        std::string lastPath = request.getRequestBody().getTempFile().path();
        response.addHeader("Location", lastPath);
        response.setFilePath(filePath);
        response.setContentType(_getMimeType(filePath));
        response.setFileSize(static_cast<size_t>(fileStat.st_size));
        response.addHeader("Connection", "close");
        return response;
    }
    else if (method == "DELETE")
    {
        std::string file = ResponseHandler::_buildFilePath(uri , root, location);
        conn->getUpload();
        Request::treatUploadLocation(conn);
        std::string locUpload = conn->uploadLocation;

        locUpload = locUpload + "/" + file.substr((file.rfind("/") + 1));
        std::cout << locUpload << std::endl;
        struct stat fileStat;
        if (stat(locUpload.c_str(), &fileStat) != 0) {
            return ErrorResponse::createNotFoundResponse(conn);
        }
        if (S_ISDIR(fileStat.st_mode)) {
            return ErrorResponse::createForbiddenResponse(conn);
        }
        if (std::remove(locUpload.c_str()) != 0) {
            return ErrorResponse::createInternalErrorResponse(conn);
        }
        Response response(204);
        response.addHeader("Content-Length", "0");
        return response;
    }
    return ErrorResponse::createInternalErrorResponse(conn);
}
