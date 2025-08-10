#include "../include/ResponseSender.hpp"
#include "../include/ErrorResponse.hpp"
#include "../include/CgiHandler.hpp"
#include "../include/ResponseHandler.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/socket.h>

#define EIGHT_KB 8192

bool ResponseSender::handleEpollOut(Connection* conn, int epollFd, std::vector<Connection*>& connections) {
    if (!conn->req) {
        return false;
    }

    try {
        if (conn->fileSendState == 0) {
            if (conn->cgiExecuted && !conn->cgiCompleted) {
                CgiHandler::waitCgi(conn);
                
                if (!conn->cgiCompleted) {
                    CgiHandler::readCgiOutput(conn);
                }
                
                if (!conn->cgiCompleted) {
                    // << "[ResponseSender] CGI still running for fd " << conn->fd 
                            //   << ", allowing other connections to proceed" << std::endl;
                    return true;
                }
                // << "[ResponseSender] CGI completed for fd " << conn->fd << std::endl;
            } else {
                // << "[ResponseSender] CGI state - executed: " << conn->cgiExecuted 
                        //   << ", completed: " << conn->cgiCompleted << std::endl;
            }

            Response* currentResponse = &conn->res;
            if (conn->cgiExecuted && conn->cgiCompleted) {
                // << "[ResponseSender] Using CGI response" << std::endl;
                conn->cgiResponse = CgiHandler::returnCgiResponse(conn);
                currentResponse = &conn->cgiResponse;
            } else if (conn->cgiExecuted && !conn->cgiCompleted) {
                // << "[ResponseSender] CGI is still running, waiting..." << std::endl;
                return true;
            } else {
                // << "[ResponseSender] Processing regular/new request" << std::endl;
                conn->res = ResponseHandler::handleRequest(conn);
                currentResponse = &conn->res;
                
                if (conn->cgiExecuted && !conn->cgiCompleted) {
                    // << "[ResponseSender] CGI started, will check completion on next EPOLLOUT" << std::endl;
                    return true;
                }
            }

            // << "[ResponseSender] Response status: " << currentResponse->getStatusCode() 
                    //   << ", filePath: " << currentResponse->getFilePath() 
                    //   << ", fileSize: " << currentResponse->getFileSize() << std::endl;

            if (!sendHeaders(conn, currentResponse, epollFd, connections)) {
                return false;
            }

            if (!currentResponse->getFilePath().empty()) {
                conn->fileFd = open(currentResponse->getFilePath().c_str(), O_RDONLY);
                if (conn->fileFd == -1) {
                    handleConnectionError(conn, connections, epollFd, "File open error");
                    return false;
                }
                conn->fileSendOffset = 0;
                conn->fileSendState = 1;
                return true;
            } else {
                conn->fileSendState = 3;
            }
        }

        if (conn->fileSendState == 1) {
            Response* currentResponse = &conn->res;
            if (conn->cgiExecuted && conn->cgiCompleted) {
                currentResponse = &conn->cgiResponse;
            }

            if (!sendFileBody(conn, currentResponse, epollFd, connections)) {
                return false;
            }
        }

        if (conn->fileSendState == 3) {
            conn->closeConnection(conn, connections, epollFd);
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        // << "Exception in response handling: " << e.what() << std::endl;
        handleConnectionError(conn, connections, epollFd, "Response handling exception");
        return false;
    }
}



void	ResponseSender::handleConnectionError(Connection* conn, std::vector<Connection*>& connections, int epollFd,  const std::string& errorMessage)
{	
	// << "Connection errot for fd " << conn->fd << ": " << errorMessage << std::endl;
    if (conn->req) {
        std::string completeResponse = Response::createErrorResponse(200, errorMessage);
        send(conn->fd, completeResponse.c_str(), completeResponse.size(), MSG_NOSIGNAL);
    }
	
	conn->closeConnection(conn, connections, epollFd);
}


bool ResponseSender::sendHeaders(Connection* conn, Response* response, int epollFd, std::vector<Connection*>& connections) {
    std::string responseStr = response->build();
    // << "response headers\n";
    // << CYAN <<  responseStr << RESET << std::endl;
    ssize_t sent = send(conn->fd, responseStr.c_str(), responseStr.size(), MSG_NOSIGNAL);
    
    if (sent == -1) {
        handleConnectionError(conn, connections, epollFd, "Header send error");
        return false;
    }
    
    conn->lastActivityTime = time(NULL);
    return true;
}

bool ResponseSender::sendFileBody(Connection* conn, Response* response, int epollFd, std::vector<Connection*>& connections) {
    char fileBuf[EIGHT_KB];
    // << " *************************body sent\n";
    if (lseek(conn->fileFd, conn->fileSendOffset, SEEK_SET) == -1) {
        close(conn->fileFd);
        conn->fileFd = -1;
        handleConnectionError(conn, connections, epollFd, "File seek error");
        return false;
    }
    ssize_t bytesRead = read(conn->fileFd, fileBuf, sizeof(fileBuf) - 1);
    if (bytesRead == 0) {
        close(conn->fileFd);
        conn->fileFd = -1;
        conn->fileSendState = 3;
        return true;
    }
    else if (bytesRead < 0) {
        close(conn->fileFd);
        conn->fileFd = -1;
        handleConnectionError(conn, connections, epollFd, "File send error");
        return false;
    }
    else {
        ssize_t bytesSent = send(conn->fd, fileBuf, bytesRead, MSG_NOSIGNAL);
        if (bytesSent == -1) {
            conn->fileSendState = 3;
            close(conn->fileFd);
            conn->fileFd = -1;
            return true;
        }
        conn->lastActivityTime = time(NULL);
        conn->fileSendOffset += bytesSent;
        if (conn->fileSendOffset >= (ssize_t)response->getFileSize()) {
            close(conn->fileFd);
            conn->fileFd = -1;
            conn->fileSendState = 3;
        }
        return true;
    }
}
