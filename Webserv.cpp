#include <map>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctime>
#include "includes/Request.hpp"

#define PORT            8080
#define MAX_EVENTS      10
#define BUFFER_SIZE     4096
#define REQUEST_TIMEOUT 30

struct ClientState {
    std::string buffer;
    Request request;
    time_t lastActivity;
    
    // Explicit constructor
    ClientState() : buffer(), request(), lastActivity(0) {}
    
    // Manual copy constructor
    ClientState(const ClientState& other) :
        buffer(other.buffer),
        request(),  // Request copy is forbidden, so we create new empty one
        lastActivity(other.lastActivity) {}
};

int makeSocketNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); return 1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen"); close(listen_fd); return 1;
    }

    if (makeSocketNonBlocking(listen_fd) < 0) {
        perror("fcntl"); close(listen_fd); return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1"); close(listen_fd); return 1;
    }

    epoll_event event = {};
    event.events = EPOLLIN;
    event.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        perror("epoll_ctl"); close(listen_fd); close(epoll_fd); return 1;
    }

    epoll_event events[MAX_EVENTS];
    std::map<int, ClientState> clients;

    // << "Server listening on port " << PORT << "...\n";

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                if (makeSocketNonBlocking(client_fd) < 0) {
                    perror("fcntl");
                    close(client_fd);
                    continue;
                }

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                    perror("epoll_ctl");
                    close(client_fd);
                    continue;
                }

                // Initialize new client state directly
                ClientState new_client;
                new_client.lastActivity = time(NULL);
                clients.insert(std::make_pair(client_fd, new_client));
                
                // << "New connection: " << client_fd << std::endl;
            } else {
                time_t now = time(NULL);
                if (now - clients[fd].lastActivity > REQUEST_TIMEOUT) {
                    // << "Connection " << fd << " timed out\n";
                    close(fd);
                    clients.erase(fd);
                    continue;
                }

                clients[fd].lastActivity = now;
                
                char chunk[BUFFER_SIZE];
                ssize_t bytesRead = recv(fd, chunk, sizeof(chunk), 0);

                if (bytesRead <= 0) {
                    if (bytesRead == 0) {
                        // << "Connection " << fd << " closed by client\n";
                    } else {
                        perror("recv");
                    }
                    close(fd);
                    clients.erase(fd);
                    continue;
                }

                clients[fd].buffer.append(chunk, bytesRead);
                bool parsed = clients[fd].request.parseFromBuffer(clients[fd].buffer);

                if (parsed) {
                    // << "\n--- Received Request ---\n";
                    // << clients[fd].request << std::endl;
                    
                    clients[fd].request.sendResponse(fd);
                    close(fd);
                    clients.erase(fd);
                } else if (clients[fd].request.state != 0) {
                    std::cerr << "Error parsing request: " << clients[fd].request.state << std::endl;
                    clients[fd].request.sendResponse(fd);
                    close(fd);
                    clients.erase(fd);
                } else {
                    // << "Didn't enter here!\n";
                    // << "Request:\n" 
                        << clients[fd].request << std::endl;
                }
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}