#pragma once
#include <string>
class DirectoryListing {
public:
    static std::string generate(const std::string& path, const std::string& uri);
}; 