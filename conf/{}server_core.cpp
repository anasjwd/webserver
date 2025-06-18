#include <sys/socket.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_EVENTS 512
#define BACKLOG 511

unsigned int formatAddress(char* host);
{
	// get each frag in a var
	// do htonl((first << 24) | (second << 16) | (third << 8) | last);
	// return it
}

bool createListeningSocket(char* host, unsigned int port, int epfd)
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrLen;
	struct poll_event pev, pevs[MAX_EVENTS];
	int optval = 1;

	sockfd = sockets(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cerr << "Error: failed to create socket\n";
		return ( -1 );
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		std::cerr << "Error: failed to set SO_REUSEADDR\n";
		close(sockfd);
		return ( -1 );
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = formatAddress(host);
	addr.sin_port = htonl(port);
	addrLen = sizeof(addr);
	if (bind(sockfd, (struct sockaddr*)&sockaddr, addrlen) < 0)
	{
		std::cerr << "Error: failed to bind to port\n";
		close(sockfd);
		return ( -1 );
	}
	if (listen(sockfd, BACKLOG) < 0)
	{
		std::cerr << "Error: failed to listen on socket\n";
		cloase(sockfd);
		return ( -1 );
	}
	pev.events = EPOLLIN;
	pev.data.fd = sockfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &pev) == -1)
	{
		std::cerr << "Error: failed to add socket to epoll\n";
		close(sockfd);
		return ( -1 );
	}
	return (sockfd);
}

bool createListeningSockets(
		std::map<char*, int> sockAddr,
		std::vector<unsigned int> sockets)
{
	int sockfd;
	int epollfd;
	std::map<char*, unsigned int>::iterator it;

	epollfd = epoll_create(1);
	if (epollfd == -1)
	{
		std::cerr << "Error: failed to create epoll instance\n";
		return ( 1 );
	}
	for (it = sockAddr.begin(); it != sockAddr.end(); ++it)
	{
		createListeningSocket(it->first,)
	}
}

int main(int ac, char** av)
{
	Http* http;
	std::map<char*, int> sockAddr;
	std::vector<unsigned int> sockets;

	if (ac < 2)
	{
		std::cerr << "Error: config file missing.\n";
		return ( 1 );
	}
	http = parseConfig(av[1]);
	getSockAddr(http, sockAddr);
	createListeningSockets(sockAddr, sockets);
	monitorSockets(sockets);

	//loop
	
	return ( 0 );
}

/*	while (idx < size)
	{
		if (http.directives[idx].getType() == SERVER)
		{
			serverHolder = &http.directives[idx];
			serverSize = serverHolder->directives.size();
			jdx = 0;
			while (jdx < serverSize)
			{
				if (serverHolder->directives[jdx].getType() == LISTEN)
				{
					sockfd = createListeningSocket(serverHolder->directives[jdx], epollfd);
					if (sockfd == -1)
						return ( 1 );
					sockets.push_back(sockfd);
				}
				++jdx;
			}
		}
		++idx;
	}
*/
