#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <utility>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
#include "cfg_parser.hpp"

#define MAX_EVENTS 512
#define BACKLOG 511
#define NONESSENTIAL 101

typedef std::pair<char*, int> IpPortKey;

unsigned int formatAddress(char* host)
{
 	std::stringstream ss(host);
	std::stringstream holder;
	std::string fragment;
	int num[4];
	int idx = 0;

    while (std::getline(ss, fragment, '.')) {
		holder << fragment;
		holder >> num[idx];
        std::cout << num[idx] << std::endl;
		holder.clear();
		++idx;
    }
	unsigned int ip = htonl((num[0] << 24) | (num[1] << 16) | (num[2] << 8) | num[3]);
	return ( ip );
}

int createListeningSocket(char* host, unsigned int port, int epfd)
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrLen;
	struct epoll_event pev;
	int optval = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = formatAddress(host);
	addr.sin_port = htons(port);
	addrLen = sizeof(addr);
	if (bind(sockfd, (struct sockaddr*)&addr, addrLen) < 0)
	{
		std::cerr << "Error: failed to bind to port\n";
		close(sockfd);
		return ( -1 );
	}
	if (listen(sockfd, BACKLOG) < 0)
	{
		std::cerr << "Error: failed to listen on socket\n";
		close(sockfd);
		return ( -1 );
	}
	memset(&pev, 0, sizeof(pev));
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
		std::map<IpPortKey, int>& sockAddr,
		std::vector<int>& sockets, int epollfd)
{
	int sockfd;
	std::map<IpPortKey, int>::iterator it;

	for (it = sockAddr.begin(); it != sockAddr.end(); ++it)
	{
		sockfd = createListeningSocket(it->first.first, it->first.second, epollfd);
		if (sockfd == -1)
			return ( 1 );
		sockets.push_back(sockfd);
	}
	return ( 0 );
}

void getSockAddr(Http* http, std::map<IpPortKey, int>& sockAddr)
{
	unsigned int size = http->directives.size();
	unsigned int idx = 0;
	unsigned int jdx;
	Server* serverHolder;
	Listen* listenHolder;
	unsigned int serverSize;

	while (idx < size)
	{
		if (http->directives[idx]->getType() == SERVER)
		{
			serverHolder = dynamic_cast<Server*>(http->directives[idx]);
			serverSize = serverHolder->directives.size();
			jdx = 0;
			while (jdx < serverSize)
			{
				if (serverHolder->directives[jdx]->getType() == LISTEN)
				{
					listenHolder = dynamic_cast<Listen*>(serverHolder->directives[jdx]);
					sockAddr[IpPortKey(listenHolder->getHost(), listenHolder->getPort())] = NONESSENTIAL;
				}
				++jdx;
			}
		}
		++idx;
	}
}

void closeSockets(std::vector<int>& sockets)
{
	unsigned int size = sockets.size();
	
	for (unsigned int idx = 0; idx < size; idx++)
		close(sockets[idx]);
}

void printMap(std::map<IpPortKey, int>& sockAddr)
{
	std::map<IpPortKey, int>::iterator it;

	for (it = sockAddr.begin(); it != sockAddr.end(); ++it)
		std::cout << "host: " << it->first.first << ", port " << it->first.second << std::endl;
}

int main(int ac, char** av)
{
	Http* http;
	std::map<IpPortKey, int> sockAddr;
	std::vector<int> sockets;
	int epollfd;

	if (ac < 2)
	{
		std::cerr << "Error: config file missing.\n";
		return ( 1 );
	}
	http = parseConfig(av[1]);
	getSockAddr(http, sockAddr);
	epollfd = epoll_create(1);
	if (epollfd == -1)
	{
		std::cerr << "Error: failed to create epoll instance\n";
		closeSockets(sockets);
		return ( 1 );
	}
	createListeningSockets(sockAddr, sockets, epollfd);
	printMap(sockAddr);
	//serverLoop(http, sockets, epollfd);
	closeSockets(sockets);
	close(epollfd);
	return ( 0 );
}

