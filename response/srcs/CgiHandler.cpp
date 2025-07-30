#include "../include/CgiHandler.hpp"
#include "../include/ErrorResponse.hpp"
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
# include "../../conf/ServerName.hpp"
# include "../../conf/Listen.hpp"

// Manual mapping of CGI extensions to interpreters
static std::map<std::string, std::string> cgiInterpreters;

static void initCgiInterpreters() {
    if (cgiInterpreters.empty()) {
        cgiInterpreters[".py"] = "/usr/bin/python3";
        cgiInterpreters[".php"] = "/usr/bin/php";
        cgiInterpreters[".pl"] = "/usr/bin/perl";
        cgiInterpreters[".sh"] = "/bin/bash";
    }
}

Response CgiHandler::executeCgi(Connection* conn, const std::string& scriptPath) {
    initCgiInterpreters();
    
    // Check if file exists and is executable
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0) {
        return ErrorResponse::createNotFoundResponse(conn);
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        return ErrorResponse::createForbiddenResponse(conn);
    }
    
    // Check if file is executable
    if (access(scriptPath.c_str(), X_OK) != 0) {
        return ErrorResponse::createForbiddenResponse(conn);
    }
    
    // Get file extension
    size_t dotPos = scriptPath.rfind('.');
    if (dotPos == std::string::npos) {
        return ErrorResponse::createNotFoundResponse(conn);
    }
    
    std::string extension = scriptPath.substr(dotPos);
    if (cgiInterpreters.find(extension) == cgiInterpreters.end()) {
        return ErrorResponse::createNotFoundResponse(conn);
    }
    
    std::string interpreter = cgiInterpreters[extension];


    std::cout << RED  <<  "script path " <<  scriptPath  << RESET << std::endl;
    std::cout << RED  <<  "interpreter " <<  interpreter  << RESET << std::endl;
    
    // Create pipes for communication
    int pipeToChild[2];
    int pipeFromChild[2];
    
    if (pipe(pipeToChild) == -1 || pipe(pipeFromChild) == -1) {
        return ErrorResponse::createInternalErrorResponse(conn);
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(pipeToChild[0]);
        close(pipeToChild[1]);
        close(pipeFromChild[0]);
        close(pipeFromChild[1]);
        return ErrorResponse::createInternalErrorResponse(conn);
    }
    
    if (pid == 0) {
        // Child process
        close(pipeToChild[1]);  // Close write end
        close(pipeFromChild[0]); // Close read end
        
        // Redirect stdin/stdout
        dup2(pipeToChild[0], STDIN_FILENO);
        dup2(pipeFromChild[1], STDOUT_FILENO);
        
        // Build environment variables
        std::map<std::string, std::string> env = buildEnvironment(conn, scriptPath);
        
        // Convert environment to char* array
        std::vector<std::string> envStrings;
        std::vector<char*> envArray;
        
        for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
            std::string envVar = it->first + "=" + it->second;
            envStrings.push_back(envVar);
        }
        
        for (size_t i = 0; i < envStrings.size(); ++i) {
            envArray.push_back(const_cast<char*>(envStrings[i].c_str()));
        }
        envArray.push_back(NULL);
        
        // Change to script directory
        std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
        std::string scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
        if (!scriptDir.empty()) {
            chdir(scriptDir.c_str());
        }
        
        // Execute the script
        char* args[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptName.c_str()), NULL};
        execve(interpreter.c_str(), args, envArray.data());
        
        // If execve fails, exit with error
        exit(1);
    } else {
        // Parent process
        close(pipeToChild[0]);  // Close read end
        close(pipeFromChild[1]); // Close write end
        
        // Send request body to CGI script if it's a POST request
        if (conn->req && conn->req->getRequestLine().getMethod() == "POST") {
            // Read from the temporary file that contains the request body
            std::string tempFile = conn->req->getRequestBody().getTempFile().path();
            if (!tempFile.empty()) {
                std::ifstream bodyFile(tempFile.c_str());
                if (bodyFile.is_open()) {
                    std::string body((std::istreambuf_iterator<char>(bodyFile)),
                                   std::istreambuf_iterator<char>());
                    bodyFile.close();
                    if (!body.empty()) {
                        write(pipeToChild[1], body.c_str(), body.length());
                    }
                }
            }
        }
        close(pipeToChild[1]);
        
        // Read CGI output
        std::string cgiOutput;
        char buffer[4096];
        ssize_t bytesRead;
        
        while ((bytesRead = read(pipeFromChild[0], buffer, sizeof(buffer))) > 0) {
            cgiOutput.append(buffer, bytesRead);
        }
        
        close(pipeFromChild[0]);
        
        // Wait for child process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return parseCgiResponse(cgiOutput);
        } else {
            return ErrorResponse::createInternalErrorResponse(conn);
        }
    }
}

std::map<std::string, std::string> CgiHandler::buildEnvironment(Connection* conn, const std::string& scriptPath) {
    std::map<std::string, std::string> env;
    
    const Request& request = *conn->req;
    std::string method = request.getRequestLine().getMethod();
    std::string uri = request.getRequestLine().getUri();
    
    // Required CGI environment variables
    env["SERVER_SOFTWARE"] = "WebServ/1.1";
    env["SERVER_NAME"] = getServerName(conn);
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_PORT"] = getServerPort(conn);
    env["REQUEST_METHOD"] = method;
    env["PATH_TRANSLATED"] = scriptPath;
    env["SCRIPT_NAME"] = uri.substr(0, uri.find('?'));
    env["QUERY_STRING"] = getQueryString(request.getRequestLine().getQueryParams());
    // env["HTTP_COOKIE"] = "num=15";
    // env["REMOTE_ADDR"] = getClientIp(conn->fd);
    // env["REMOTE_HOST"] = getClientIp(conn->fd);
    
// Content-related variables
    if (method == "POST") {
        env["CONTENT_TYPE"] = request.getRequestHeaders().getHeaderValue("content-type");
        env["CONTENT_LENGTH"] = request.getRequestHeaders().getHeaderValue("content-length");
    }
    
    // HTTP headers as environment variables
    const std::map<std::string, std::string>& headers = request.getRequestHeaders().getHeadersMap();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::string envName = "HTTP_" + it->first;
        std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
        std::replace(envName.begin(), envName.end(), '-', '_');
        env[envName] = it->second;
    }
    
    return env;
}

Response CgiHandler::parseCgiResponse(const std::string& cgiOutput) {
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
    }
    
    if (headerEnd == std::string::npos) {
        // No headers found, treat entire output as body
        Response response(200);
        response.setContentType("text/plain");
        response.setFileBody("/tmp/cgi_output");
        
        std::ofstream tempFile("/tmp/cgi_output");
        tempFile << cgiOutput;
        tempFile.close();
        
        return response;
    }
    
    std::string headers = cgiOutput.substr(0, headerEnd);
    std::string body = cgiOutput.substr(headerEnd + (cgiOutput.find("\r\n\r\n") != std::string::npos ? 4 : 2));
    
    Response response(200);
    
    // Parse CGI headers
    std::istringstream headerStream(headers);
    std::string line;
    while (std::getline(headerStream, line) && !line.empty()) {
        if (line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remove leading spaces from value
            while (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            
            if (name == "Status") {
                // Parse status line (e.g., "200 OK")
                size_t spacePos = value.find(' ');
                if (spacePos != std::string::npos) {
                    int statusCode = std::atoi(value.substr(0, spacePos).c_str());
                    response.setStatus(statusCode);
                }
            } else if (name == "Content-Type") {
                response.setContentType(value);
            } else if (name == "Location") {
                response.addHeader("Location", value);
            } else {
                response.addHeader(name, value);
            }
        }
    }
    
    // Set body
    if (!body.empty()) {
        std::ofstream tempFile("/tmp/cgi_output");
        tempFile << body;
        tempFile.close();
        response.setFileBody("/tmp/cgi_output");
    }
    
    return response;
}

std::string CgiHandler::getQueryString(const std::map<std::string, std::string>& queryParams) {
    if (queryParams.empty()) {
        return "";
    }
    
    std::string queryString;
    for (std::map<std::string, std::string>::const_iterator it = queryParams.begin(); it != queryParams.end(); ++it) {
        if (!queryString.empty()) {
            queryString += "&";
        }
        queryString += it->first + "=" + it->second;
    }
    return queryString;
}


std::string CgiHandler::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            if (isxdigit(str[i+1]) && isxdigit(str[i+2])) {
                char hex[3] = {str[i+1], str[i+2], '\0'};
                char *end;
                unsigned long val = strtoul(hex, &end, 16);
                if (*end == '\0') {
                    result += static_cast<char>(val);
                    i += 2;
                    continue;
                }
            }
        } else if (str[i] == '+') {
            result += ' ';
            continue;
        }
        result += str[i];
    }
    return result;
}

std::string CgiHandler::getClientIp(int fd) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    if (getpeername(fd, (struct sockaddr*)&addr, &addrLen) == 0) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        return std::string(ip);
    }
    return "127.0.0.1";
}

std::string CgiHandler::getServerPort(Connection* conn) {
    if (conn && conn->conServer) {
        for (std::vector<IDirective*>::const_iterator it = conn->conServer->directives.begin(); 
             it != conn->conServer->directives.end(); ++it) {
            if ((*it)->getType() == LISTEN) {
	            Listen* listen = static_cast<Listen*>(*it);
                if (listen)
                {
                    std::ostringstream oss;
                    oss << listen->getPort();
                    return  oss.str(); 
                }
            }
        }
    }
    return "8080";
}

std::string CgiHandler::getServerName(Connection* conn) {
    if (conn && conn->conServer) {
        for (std::vector<IDirective*>::const_iterator it = conn->conServer->directives.begin(); 
             it != conn->conServer->directives.end(); ++it) {
            if ((*it)->getType() == SERVER_NAME) {
                ServerName *sn = static_cast<ServerName*>(*it);
				if (sn) {
					char** names = sn->getServerNames();
					if (names)
					    return  names[0];
                }
            }
        }
    }
    return "localhost";
} 
