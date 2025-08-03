#include "../include/CgiHandler.hpp"
#include "../include/ErrorResponse.hpp"
#include <cstddef>
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
#include <signal.h>
#include <sstream>
# include "../../conf/ServerName.hpp"
# include "../../conf/Listen.hpp"

static std::map<std::string, std::string> cgiInterpreters;

static void initCgiInterpreters() {
    if (cgiInterpreters.empty()) {
        cgiInterpreters[".py"] = "/usr/bin/python3";
        cgiInterpreters[".php"] = "/usr/bin/php-cgi";
        cgiInterpreters[".sh"] = "/bin/bash";
    }
}

Response CgiHandler::executeCgi(Connection* conn, const std::string& scriptPath) {
    initCgiInterpreters();
    
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0) {
        std::cout << RED << "file not opened" << RESET << std::endl;
        return ErrorResponse::createNotFoundResponse(conn);
    }
    if (!S_ISREG(fileStat.st_mode)) {
        std::cout << RED << "file not a regular file" << RESET << std::endl;
        return ErrorResponse::createForbiddenResponse(conn);
    }
    if (access(scriptPath.c_str(), R_OK) != 0) {
        std::cout << RED << "file not readable" << RESET << std::endl;
        return ErrorResponse::createForbiddenResponse(conn);
    }
    
    size_t dotPos = scriptPath.rfind('.');
    if (dotPos == std::string::npos) {
        return ErrorResponse::createNotFoundResponse(conn);
    }
    std::string extension = scriptPath.substr(dotPos);
    if (cgiInterpreters.find(extension) == cgiInterpreters.end()) {
        return ErrorResponse::createNotFoundResponse(conn);
    }
    
    std::string interpreter = cgiInterpreters[extension];
    
    if (pipe(conn->cgiPipeFromChild) == -1) {
        return ErrorResponse::createInternalErrorResponse(conn);
    }
    
    bool isPost = conn->req->getRequestLine().getMethod() == "POST";
    if (isPost && pipe(conn->cgiPipeToChild) == -1) {
        close(conn->cgiPipeFromChild[0]);
        close(conn->cgiPipeFromChild[1]);
        return ErrorResponse::createInternalErrorResponse(conn);
    }
    
    conn->cgiPid = fork();
    if (conn->cgiPid == -1) {
        close(conn->cgiPipeFromChild[0]);
        close(conn->cgiPipeFromChild[1]);
        if (isPost) {
            close(conn->cgiPipeToChild[0]);
            close(conn->cgiPipeToChild[1]);
        }
        return ErrorResponse::createInternalErrorResponse(conn);
    }
    
    if (conn->cgiPid == 0) {
        close(conn->cgiPipeFromChild[0]);
        
        dup2(conn->cgiPipeFromChild[1], STDOUT_FILENO);
        close(conn->cgiPipeFromChild[1]);
        
        if (isPost) {
            close(conn->cgiPipeToChild[1]);
            dup2(conn->cgiPipeToChild[0], STDIN_FILENO);
            close(conn->cgiPipeToChild[0]);
        }
        
        std::map<std::string, std::string> env = buildEnvironment(conn, scriptPath);
        
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
        
        char* args[3];
        if (extension == ".php") {
            args[0] = const_cast<char*>(interpreter.c_str());
            args[1] = const_cast<char*>(scriptPath.c_str());
            args[2] = NULL;
        } else {
            std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
            std::string scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
            
            if (!scriptDir.empty()) {
                chdir(scriptDir.c_str());
            }
            
            args[0] = const_cast<char*>(interpreter.c_str());
            args[1] = const_cast<char*>(scriptName.c_str());
            args[2] = NULL;
        }
        
        execve(interpreter.c_str(), args, envArray.data());
        
        exit(1);
    } else {
        close(conn->cgiPipeFromChild[1]);
        
        if (isPost) {
            close(conn->cgiPipeToChild[0]);
            close(conn->cgiPipeToChild[1]);
        }
        
        conn->cgiExecuted = true;
        conn->cgiCompleted = false;
        conn->cgiReadState = 0;
        conn->cgiHeaders.clear();
        conn->cgiBody.clear();
        conn->cgiStartTime = time(NULL);
        
        return Response(200);
    }
}

void CgiHandler::waitCgi(Connection* conn) {
    if (!conn->cgiExecuted || conn->cgiCompleted) {
        return;
    }

    time_t currentTime = time(NULL);
    if (currentTime - conn->cgiStartTime > CGI_TIMEOUT) {
        if (conn->cgiPid > 0) {
            kill(conn->cgiPid, SIGTERM);
            usleep(100000);
            kill(conn->cgiPid, SIGKILL);
        }
        conn->cgiCompleted = true;
        close(conn->cgiPipeFromChild[0]);
        return;
    }

    int status;
    int result = waitpid(conn->cgiPid, &status, WNOHANG);
        
    if (result == conn->cgiPid) {
        conn->cgiCompleted = true;
        close(conn->cgiPipeFromChild[0]);
    } else if (result == -1) {
        conn->cgiCompleted = true;
        close(conn->cgiPipeFromChild[0]);
    }
}

void CgiHandler::readCgiOutput(Connection* conn) {
    if (!conn->cgiExecuted || conn->cgiCompleted) {
        return;
    }
    
    char buffer[8192];
    ssize_t bytesRead = read(conn->cgiPipeFromChild[0], buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << YELLOW << "#######################################" << RESET << std::endl;
        std::cout << buffer << std::endl;
        std::cout << YELLOW << "#######################################" << RESET << std::endl;
        conn->cgiOutput += std::string(buffer, bytesRead);
        
        if (conn->cgiReadState == 0) {
            size_t headerEnd = conn->cgiOutput.find("\r\n\r\n");
            bool isCRLF = true;
            if (headerEnd == std::string::npos) {
                headerEnd = conn->cgiOutput.find("\n\n");
                isCRLF = false;
            }
            
            if (headerEnd != std::string::npos) {
                conn->cgiHeaders = conn->cgiOutput.substr(0, headerEnd);
                conn->cgiBody = conn->cgiOutput.substr(headerEnd + (isCRLF ? 4 : 2));
                conn->cgiReadState = 1;
            }
        } else {
            conn->cgiBody += std::string(buffer, bytesRead);
        }
    } else if (bytesRead == 0) {
        conn->cgiCompleted = true;
        close(conn->cgiPipeFromChild[0]);
    } else if (bytesRead == -1) {
        if (conn->cgiPid > 0) {
            kill(conn->cgiPid, SIGTERM);
            usleep(100000);
            kill(conn->cgiPid, SIGKILL);
        }
        conn->cgiCompleted = true;
        close(conn->cgiPipeFromChild[0]);
    }
}

Response CgiHandler::returnCgiResponse(Connection* conn) {
    if (!conn->cgiCompleted) {
        return Response(200);
    }
    
    if (conn->cgiHeaders.empty()) {
        conn->cgiResponse.setStatus(200);
        
        std::ofstream tempFile("/tmp/cgi_output");
        tempFile << conn->cgiOutput;
        tempFile.close();
        
        conn->cgiResponse.setFileBody("/tmp/cgi_output");
        return conn->cgiResponse;
    }
    
    std::istringstream headerStream(conn->cgiHeaders);
    std::string line;
    bool hasContentType = false;
    bool hasStatus = false;
    
    while (std::getline(headerStream, line)) {
        if (line.empty() || line == "\r") break;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                value = value.substr(1);
            }
            
            if (!value.empty() && value[value.length() - 1] == '\r') {
                value = value.substr(0, value.length() - 1);
            }
            
            if (name == "Status") {
                std::string statusStr = value.substr(0, 3);
                int statusCode = atoi(statusStr.c_str());
                if (statusCode > 0) {
                    conn->cgiResponse.setStatus(statusCode);
                    hasStatus = true;
                }
            } else if (name == "Content-Type") {
                conn->cgiResponse.setContentType(value);
                hasContentType = true;
            } else {
                conn->cgiResponse.addHeader(name, value);
            }
        }
    }
    
    if (!hasStatus) {
        conn->cgiResponse.setStatus(200);
    }
    if (!hasContentType) {
        conn->cgiResponse.setContentType("text/html");
    }
    conn->cgiResponse.addHeader("Connection", "close");
    
    if (!conn->cgiBody.empty()) {
        std::ofstream tempFile("/tmp/cgi_output");
        tempFile << conn->cgiBody;
        tempFile.close();
        conn->cgiResponse.setFileBody("/tmp/cgi_output");
    }
    
    return conn->cgiResponse;
}

std::map<std::string, std::string> CgiHandler::buildEnvironment(Connection* conn, const std::string& scriptPath) {
    std::map<std::string, std::string> env;
    
    const Request& request = *conn->req;
    std::string method = request.getRequestLine().getMethod();
    std::string uri = request.getRequestLine().getUri();
    
    env["REDIRECT_STATUS"] = "200";
    env["SERVER_SOFTWARE"] = "WebServ/1.1";
    env["SERVER_NAME"] = getServerName(conn);
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_PORT"] = getServerPort(conn);
    env["REQUEST_METHOD"] = method;
    env["PATH_TRANSLATED"] = scriptPath;
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = uri.substr(0, uri.find('?'));
    env["REQUEST_URI"] = uri;
    env["DOCUMENT_ROOT"] = scriptPath.substr(0, scriptPath.find_last_of('/'));
    env["QUERY_STRING"] = getQueryString(request.getRequestLine().getQueryParams());
    env["HTTP_COOKIE"] = request.getRequestHeaders().getHeaderValue("cookie");
    
    if (method == "POST") {
        env["CONTENT_TYPE"] = request.getRequestHeaders().getHeaderValue("content-type");
        env["CONTENT_LENGTH"] = request.getRequestHeaders().getHeaderValue("content-length");
        if (request.getRequestBody().isMultipart() && request.getRequestBody().getUploadHandler().isOpen()) 
            env["UPLOADED_FILE_PATH"] = request.getRequestBody().getUploadHandler().path();
        else
            env["UPLOADED_FILE_PATH"] = request.getRequestBody().getTempFile().path();     
    }
    
    const std::map<std::string, std::string>& headers = request.getRequestHeaders().getHeadersMap();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::string envName = "HTTP_" + it->first;
        std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
        std::replace(envName.begin(), envName.end(), '-', '_');
        env[envName] = it->second;
    }
    return env;
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