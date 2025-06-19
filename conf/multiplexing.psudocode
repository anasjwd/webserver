int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, ...);
listen(server_fd, 10);

// Add listening socket to epoll
struct epoll_event events[MAX_EVENTS];

// add servers to epoll
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = server_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

while (1) {
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < n; i++) {
		// check if an the event recorded is for a server or a client
		// if its for a server you will be accepting a connection
		// if its for a client you will be reading o writing (get sure if writing needs multiplexing)
        if (events[i].data.fd == server_fd) {
            // Accept new connection
            int client_fd = accept(server_fd, NULL, NULL);
            // Monitor the new client for data by adding it to epoll
            ev.events = EPOLLIN;
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        } else {
            // Handle client data
        }
    }
}
