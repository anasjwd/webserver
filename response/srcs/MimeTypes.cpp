#include "../include/MimeTypes.hpp"
#include <map>
#include <string>
std::string MimeTypes::get(const std::string& path) {
    static std::map<std::string, std::string> mimeMap;
    if (mimeMap.empty()) {
        mimeMap[".html"] = "text/html";
        mimeMap[".htm"] = "text/html";
        mimeMap[".css"] = "text/css";
        mimeMap[".js"] = "application/javascript";
        mimeMap[".json"] = "application/json";
        mimeMap[".png"] = "image/png";
        mimeMap[".jpg"] = "image/jpeg";
        mimeMap[".jpeg"] = "image/jpeg";
        mimeMap[".gif"] = "image/gif";
        mimeMap[".svg"] = "image/svg+xml";
        mimeMap[".ico"] = "image/x-icon";
        mimeMap[".txt"] = "text/plain";
        mimeMap[".pdf"] = "application/pdf";
        mimeMap[".zip"] = "application/zip";
        mimeMap[".tar"] = "application/x-tar";
        mimeMap[".gz"] = "application/gzip";
        mimeMap[".mp3"] = "audio/mpeg";
        mimeMap[".mp4"] = "video/mp4";
        mimeMap[".xml"] = "application/xml";
    }
    size_t dot = path.rfind('.');
    if (dot != std::string::npos) {
        std::string ext = path.substr(dot);
        if (mimeMap.count(ext)) return mimeMap[ext];
    }
    return "application/octet-stream";
} 