# pragma once

# include <map>
# include <sys/epoll.h>
# include "Request.hpp"

# define	MAX_EVENTS 10
# define	BUFFER_SIZE 1024

class EpollServer
{
	private:
		int						epoll_fd;
		int						server_fd;
		std::map<int, Request>	client_requests;

		void					setNonBlocking(int fd);
		void					handleNewConnection();
		void					handleClientData(int client_fd);
	
	public:
		EpollServer(int port);
		~EpollServer();

		void					run();
};
