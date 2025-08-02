#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"
#include "../../Connection.hpp"

class CgiHandler {
public:
    static Response executeCgi(Connection* conn, const std::string& scriptPath);
    static void waitCgi(Connection* conn);
    static void readCgiOutput(Connection* conn);
    static Response returnCgiResponse(Connection* conn);
    
private:
    static std::map<std::string, std::string> buildEnvironment(Connection* conn, const std::string& scriptPath);
    static std::string getCgiInterpreter(const std::string& scriptPath, Connection* conn);
    static Response parseCgiResponse(Connection* conn, const std::string& cgiOutput);
    static std::string getQueryString(const std::map<std::string, std::string>& queryParams);
    static std::string getServerPort(Connection* conn);
    static std::string getServerName(Connection* conn);
}; 