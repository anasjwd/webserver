#include <ostream>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
#include "conf/cfg_parser.hpp"
#include "request/incs/Request.hpp"
#include <algorithm>
#include <string>

#define MAX_EVENTS 512
#define BACKLOG 511
#define NONESSENTIAL 101
#define EIGHT_KB 8192

typedef std::pair<std::string, int> IpPortKey;

std::string toString(int number)
{
	std::stringstream ss;
	ss << number;
	return ( ss.str() );
}

int createListeningSocket(const char* host, unsigned int port, int epfd)
{
	int sockfd;
	struct addrinfo hints, *res = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(host, toString(port).c_str(), &hints, &res) != 0)
	{
		std::cerr << "Error: failed to get IPv4 [" << host << "]"
		<< " with port " << port << std::endl;
		return ( -1 );
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
	{
		std::cerr << "Error: failed to create socket for host [ " << host
			<< ":" << port << " ]\n";
		freeaddrinfo(res);
		return ( -1 );
	}

	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		std::cerr << "Error: failed to set SO_REUSEADDR on host [ "
		   << host << ":" << port << " ]\n";	
		close(sockfd);
		freeaddrinfo(res);
		return ( -1 );
	}

	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
	{
		std::cerr << "Error: failed to bind socket with the host: [ "
			<< host << ":" << port << " ]\n";
		close(sockfd);
		freeaddrinfo(res);
		return ( -1 );
	}

	if (listen(sockfd, BACKLOG) < 0)
	{
		std::cerr << "Error: failed to listen on socket with the host [ "
			<< host << ":" << port << " ]\n";
		close(sockfd);
		freeaddrinfo(res);
		return ( -1 );
	}

	struct epoll_event pev;
	memset(&pev, 0, sizeof(pev));
	pev.events = EPOLLIN;
	pev.data.fd = sockfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &pev) == -1)
	{
		std::cerr << "Error: failed to add socket with the host [ " <<
			host << ":" << port << " ] to epoll\n";
		close(sockfd);
		freeaddrinfo(res);
		return ( -1 );
	}

	std::cout << "Server listening on [ " << host << ":" << port << " ]\n";
	freeaddrinfo(res);
	return (sockfd);
}

bool createListeningSockets(
		std::map<IpPortKey, int>& sockAddr,
		std::vector<int>& sockets, int epollFd)
{
	int sockfd;
	std::map<IpPortKey, int>::iterator it;

	for (it = sockAddr.begin(); it != sockAddr.end(); ++it)
	{
		sockfd = createListeningSocket(it->first.first.c_str(), it->first.second, epollFd);
		if (sockfd == -1)
			return ( false );
		sockets.push_back(sockfd);
	}
	return ( true );
}

void addUniqueListen(std::map<IpPortKey, int>& sockAddr, const char* host, int port)
{
	IpPortKey wildcard = IpPortKey("0.0.0.0", port);
	IpPortKey candidate = IpPortKey(host, port);
	std::map<IpPortKey, int>::iterator it;
	std::map<IpPortKey, int>::iterator toErase;

	if (strcmp(host, "0.0.0.0") == 0)
	{
		it = sockAddr.begin();
		while (it != sockAddr.end())
		{
			if (it->first.second == port)
			{
				toErase = it++;
				sockAddr.erase(toErase);
			}
			else
				++it;
			sockAddr[wildcard] = NONESSENTIAL;
		}
	}
	else
	{
		if (sockAddr.count(wildcard) == 0)
			sockAddr[candidate] = NONESSENTIAL;
	}
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
					addUniqueListen(sockAddr, listenHolder->getHost(), listenHolder->getPort());
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

void serverLoop(Http* http, std::vector<int>& sockets, int epollFd)
{
	struct epoll_event ev, events[MAX_EVENTS];
	int numberOfEvents;
	std::vector<int>::iterator it;
	char buff[EIGHT_KB];
	ssize_t bytes;
	Request request;

	(void)http;
	while (true)
	{
		std::cout << "State => " << ev.events << "\n";
		numberOfEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		for (int i = 0; i < numberOfEvents; i++)
		{
			it = find(sockets.begin(), sockets.end(), events[i].data.fd);
			if (it != sockets.end())
			{
				int clientFd = accept(*it, NULL, NULL);
				if (clientFd == -1)
				{
					std::cout << "Error: failed to accept a client\n";
					continue;
				}
				ev.events = EPOLLIN;
				ev.data.fd = clientFd;
				epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);
			}
			else if (events[i].events & EPOLLIN)
			{
				bytes = read(events[i].data.fd, buff, EIGHT_KB);
				if (bytes == -1)
				{
					epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
				}
				else if (bytes == 0)
				{
					epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
				}
				else
				{
					std::cout.write(buff, bytes) << std::endl;
					// TODO: CREATE LINKED LIST OF CONNECTIONS.


					request.appendToBuffer(buff, bytes);
					if (request.isRequestDone())
					{
						std::cout << "Request is done, processing response..." << std::endl;
						ev.events = EPOLLOUT;
						ev.data.fd = events[i].data.fd;
						if (epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1) {
							close(events[i].data.fd);
						}
					}
					//call request parsing
					//when parsing is done call response builder
					//when the response is built change to EPOLLOUT
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				std::cout << "We in response state\n";
				//send 8kb each time
				//keep track of how write wrote
				//if the number of written character exceeds the size
				//of the WRITE_FROM buffer delete it from epoll and close fd
				ev.events = 0;
				ev.data.fd = events[i].data.fd;
				if (epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1)
					close(events[i].data.fd);
				std::string body = "<!DOCTYPE html>\n"
                   "<html>\n"
                   "<head><title>Test Page</title></head>\n"
                   "<body>\n"
                   "  <h1>Hello from my C++ server!</h1>\n"
                   "  <p>This is a test web page.</p>\n"
                   "</body>\n"
                   "</html>\n";

				std::ostringstream response;
				response << "HTTP/1.1 200 OK\r\n"
						<< "Content-Type: text/html\r\n"
						<< "Content-Length: " << body.size() << "\r\n"
						<< "\r\n"
						<< body;

				send(events[i].data.fd, response.str().c_str(), response.str().size(), 0);
			}
		}
	}
}

int main(int ac, char** av)
{
	Http* http;
	std::map<IpPortKey, int> sockAddr;
	std::vector<int> sockets;
	int epollFd;

	if (ac < 2)
	{
		std::cerr << "Error: config file missing.\n";
		return ( 1 );
	}
	http = parseConfig(av[1]);
	if (http == NULL)
		return ( 1 );
	getSockAddr(http, sockAddr);
	epollFd = epoll_create(1);
	if (epollFd == -1)
	{
		std::cerr << "Error: failed to create epoll instance\n";
		closeSockets(sockets);
		return ( 1 );
	}
	if (createListeningSockets(sockAddr, sockets, epollFd) == false)
	{
		closeSockets(sockets);
		close(epollFd);
		delete http;
		return ( 1 );
	}
	serverLoop(http, sockets, epollFd);
	closeSockets(sockets);
	close(epollFd);
	delete http;
	return ( 0 );
}

