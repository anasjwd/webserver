# include <fcntl.h>
# include <cerrno>
# include <cstring>
# include <unistd.h>
# include <iostream>
# include <stdexcept>
# include <sys/socket.h>
# include <netinet/in.h>
# include "../includes/EpollServer.hpp"

EpollServer::EpollServer(int port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));

    if (listen(server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
        throw std::runtime_error("Failed to add server socket to epoll: " + std::string(strerror(errno)));
}

EpollServer::~EpollServer()
{
    close(epoll_fd);
    close(server_fd);
}

void EpollServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        throw std::runtime_error("fcntl F_GETFL failed: " + std::string(strerror(errno)));

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed: " + std::string(strerror(errno)));
}

void EpollServer::handleNewConnection()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    
    if (client_fd == -1)
    {
        std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(client_fd);

    // Add client to epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
    {
        std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
        close(client_fd);
        return;
    }

    // Initialize request for this client
    client_requests[client_fd] = Request();
}

void EpollServer::handleClientData(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        std::string request_data(buffer, bytes_read);
        
        // Process the request
        Request& request = client_requests[client_fd];
        if (request.readFromSocket(client_fd)) {
            // Request processing completed
            std::cout << "Request processed with status: " << request.state << std::endl;
            
            // Here you would typically generate and send a response
            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
            send(client_fd, response.c_str(), response.size(), 0);
            
            // Clean up
            client_requests.erase(client_fd);
            close(client_fd);
            return;
        }
    }

    if (bytes_read == 0) {
        // Connection closed by client
        client_requests.erase(client_fd);
        close(client_fd);
    } else if (bytes_read == -1 && errno != EAGAIN) {
        // Error occurred
        std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
        client_requests.erase(client_fd);
        close(client_fd);
    }
}

void EpollServer::run() {
    struct epoll_event events[MAX_EVENTS];
    
    std::cout << "Server started. Waiting for connections..." << std::endl;

    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            continue;
        }

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_fd)
                handleNewConnection();
            else
                handleClientData(events[i].data.fd);
        }
    }
}