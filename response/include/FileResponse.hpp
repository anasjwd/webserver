#pragma once
#include "Response.hpp"
#include <string>
class FileResponse {
public:
    static Response serve(const std::string& filePath, const std::string& mimeType, int statusCode = 200);
}; 