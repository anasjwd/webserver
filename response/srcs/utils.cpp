#include "../include/utils.hpp"
#include <fstream>
#include <sstream>
#include <string>

std::string loadFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return "";
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::string getMimeType(const std::string& path) {
	if (path.find(".html") != std::string::npos) return "text/html";
	if (path.find(".css") != std::string::npos) return "text/css";
	if (path.find(".js") != std::string::npos) return "application/javascript";
	if (path.find(".png") != std::string::npos) return "image/png";
	return "text/plain";
}
