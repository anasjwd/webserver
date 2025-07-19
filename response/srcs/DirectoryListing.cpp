#include "../include/DirectoryListing.hpp"
#include <dirent.h>
#include <sys/stat.h>
std::string DirectoryListing::generate(const std::string& path, const std::string& uri) {
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