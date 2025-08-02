#pragma once

#include "Response.hpp"
#include "../../Connection.hpp"
#include <sys/epoll.h>

class ResponseSender {
public:
    static bool handleEpollOut(Connection* conn, int epollFd, std::vector<Connection*>& connections);
    
private:
    static void handleConnectionError(Connection* conn, std::vector<Connection*>& connections, int epollFd, const std::string& error);
    static bool sendHeaders(Connection* conn, Response* response, int epollFd, std::vector<Connection*>& connections);
    static bool sendFileBody(Connection* conn, Response* response, int epollFd, std::vector<Connection*>& connections);
}; 