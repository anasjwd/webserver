# include <map>
# include <ctime>
# include <string>
# include <vector>
# include <fcntl.h>
# include <csignal>
# include <sstream>
# include <cstddef>
# include <ostream>
# include <netdb.h>
# include <utility>
# include <cstring>
# include <signal.h>
# include <iostream>
# include <unistd.h>
# include <algorithm>
# include <sys/types.h>
# include <arpa/inet.h>
# include <sys/epoll.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <netinet/tcp.h>
# include <sys/sendfile.h>
# include "Connection.hpp"
# include "conf/Server.hpp"
# include "conf/IDirective.hpp"
# include "conf/cfg_parser.hpp"
# include "request/incs/Request.hpp"
# include "response/include/Response.hpp"
# include "response/include/FileResponse.hpp"
# include "response/include/ErrorResponse.hpp"
# include "response/include/ResponseSender.hpp"
# include "response/include/ResponseHandler.hpp"

# define	NONESSENTIAL	101
# define	BACKLOG			511
# define	MAX_EVENTS		512
# define	EIGHT_KB		1048576

/*
	TODO:
	It takes too long for uploading files, thinking of incrementing it from 8KB to 500KB or 1MB.
	To see with jawad later: 524288, 1048576.
*/ 

bool got_singint = false;

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

void	checkForTimeouts(std::vector<Connection*>& connections, struct epoll_event ev, int epollFd, time_t& checkTime)
{
	if (connections.size() == 0)
		return ;
	std::vector<Connection*>::iterator it = connections.begin();

	while (it != connections.end())
	{
		Connection* conn = *it;
		if (conn && conn->isTimedOut() && !conn->isCgi)
		{
			std::cout << "Connection timeout for fd " << conn->fd << std::endl;
			if (conn->req)
				conn->req->setState(false, REQUEST_TIMEOUT);
			epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, &ev);
			std::cout << "Closing connection fd " << conn->fd << std::endl;
			conn->closeConnection(conn, connections, epollFd);
		}
		else if (conn && conn->isCgi && conn->isCgiTimedOut())
		{
			std::cout << "Connection cgi timeout for fd " << conn->fd << std::endl;
			Response res =  FileResponse::serve("www/error_504.html" , "text/html", 504);
			  std::string responseStr = res.build();
			// std::cout << "response headers\n";
			// std::cout << CYAN <<  responseStr << RESET << std::endl;
			send(conn->fd, responseStr.c_str(), responseStr.size(), 0);
			conn->fileFd = open(res.getFilePath().c_str(), O_RDONLY);
			char fileBuf[EIGHT_KB]; 
   			ssize_t bytesRead = read(conn->fileFd, fileBuf, sizeof(fileBuf) - 1);
			std::cout << RED  << fileBuf << RESET << std::endl;
			send(conn->fd, fileBuf, bytesRead, 0);

			epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, &ev);
			std::cout << "Closing cgi connection fd " << conn->fd << std::endl;
			conn->closeConnection(conn, connections, epollFd);
		}
		if (connections.size() == 0)
			break;
		++it;
	}
	checkTime = time(NULL);
}

void	handleConnectionError(Connection* conn, std::vector<Connection*>& connections, int epollFd, const std::string& error)
{
	std::cout << "Connection error for fd " << conn->fd << ": " << error << std::endl;
	
	// Send error response if possible
	if (conn->req) {
		conn->res = ErrorResponse::createInternalErrorResponse(conn);
		std::string responseStr = conn->res.build();
		send(conn->fd, responseStr.c_str(), responseStr.size(), 0);
	}
	
	conn->closeConnection(conn, connections, epollFd);
}

void	serverLoop(Http* http, std::vector<int>& sockets, int epollFd)
{
	int it = 0;
	Connection*					conn;
	std::vector<Connection*>	connections;
	int							numberOfEvents;
	struct epoll_event			ev, events[MAX_EVENTS];
	time_t						lastTimeoutCheck = time(NULL);

	while (true)
	{
		std::cout << "passed here for the " << it << " th time.";
		if (got_singint == true)
			return conn->freeConnections(connections);
		if (time(NULL) - lastTimeoutCheck >= 1)
			checkForTimeouts(connections, ev, epollFd, lastTimeoutCheck);

		numberOfEvents = epoll_wait(epollFd, events, MAX_EVENTS, 1000);
		int i = 0;
		for (; i < numberOfEvents; i++)
		{
			if (std::find(sockets.begin(), sockets.end(), events[i].data.fd) != sockets.end())
			{
				int clientFd = accept(events[i].data.fd, NULL, NULL);
				if (clientFd == -1)
				{
					std::cout << "Error: failed to accept a client" << std::endl;
					continue;
				}
				
				int flags = fcntl(clientFd, F_GETFL, 0);
				fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
				int optval = 1;
				setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

				connections.push_back(new Connection(clientFd));
				std::cout << RED << "Connection created!\n" << RESET;

				ev.events = EPOLLIN;
				ev.data.fd = clientFd;
				epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);
			} 
			else
			{
				conn = conn->findConnectionByFd(events[i].data.fd, connections);
				if (!conn)
					continue;

				if (events[i].events & EPOLLIN)
					conn->epollinProcess(http, conn, connections, ev, epollFd);
				else if (events[i].events & EPOLLOUT)
				{
					std::cout << "EPOLLOUT event triggered for fd " << conn->fd << std::endl;
					it++;
					if (!ResponseSender::handleEpollOut(conn, epollFd, connections)) {
						conn = NULL;
					}
				}
				else if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
				{
					std::cout << "Connection error or hangup on fd " << conn->fd << std::endl;
					conn->closeConnection(conn, connections, epollFd);
					conn = NULL;
				}
			}
		}
	}
}


void sigintHandler(int sig)
{
	(void)sig;
	got_singint = true;
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
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, sigintHandler);
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
