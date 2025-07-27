#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"
#include "../../Connection.hpp"

class CgiHandler {
public:
    static Response executeCgi(Connection* conn, const std::string& scriptPath);
    
private:
    static std::map<std::string, std::string> buildEnvironment(Connection* conn, const std::string& scriptPath);
    static std::string getCgiInterpreter(const std::string& scriptPath, Connection* conn);
    static bool isCgiScript(const std::string& scriptPath, Connection* conn);
    static Response parseCgiResponse(const std::string& cgiOutput);
    static std::string getQueryString(const std::map<std::string, std::string>& queryParams);
    static std::string urlDecode(const std::string& str);
    static std::string getClientIp(int fd);
    static std::string getServerPort(Connection* conn);
    static std::string getServerName(Connection* conn);
}; 