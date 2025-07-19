#pragma once

#include <string>

// Returns the path to the created file in tmp/response
std::string loadFile(const std::string& path);
std::string getMimeType(const std::string& path);
