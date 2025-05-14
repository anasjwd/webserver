# include <cstring>
# include <unistd.h>
# include <iostream>
# include <sys/socket.h>
# include "includes/Request.hpp"

int main()
{
    {
        int pipefd[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
        
        const char* request = 
            "GET /test?param1=value1&param2=value2 HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "User-Agent: test-client\r\n"
            "Accept: */*\r\n"
            "\r\n";
        
        write(pipefd[0], request, strlen(request));
        
        Request req;
        std::cout << "\n";
        if (req.readFromSocket(pipefd[1])) {
            std::cout << "Test 1 - GET request:" << std::endl;
            std::cout << req << std::endl;
            std::cout << "Request state: " << req.state << std::endl;
        } else {
            std::cerr << "Test 1 failed with state: " << req.state << std::endl;
        }
        
        close(pipefd[0]);
        close(pipefd[1]);
    }
    {
        int pipefd[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
        
        const char* request = 
            "POST /submit HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: 23\r\n"
            "\r\n"
            "field1=value1&field2=value2";
        
        write(pipefd[0], request, strlen(request));
        
        Request req;
        if (req.readFromSocket(pipefd[1])) {
            std::cout << "Test 2 - POST request:" << std::endl;
            std::cout << req << std::endl;
            std::cout << "Request state: " << req.state << std::endl;
        } else {
            std::cerr << "Test 2 failed with state: " << req.state << std::endl;
        }
        
        close(pipefd[0]);
        close(pipefd[1]);
    }
}
