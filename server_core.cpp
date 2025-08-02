# include <map>
# include <ctime>
# include <string>
# include <vector>
# include <fcntl.h>
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
# include "request/incs/Defines.hpp"
# include "request/incs/Request.hpp"
# include "response/include/Response.hpp"
# include "response/include/ErrorResponse.hpp"
# include "response/include/ResponseHandler.hpp"

# define	NONESSENTIAL	101
# define	MAX_EVENTS		512
# define	BACKLOG			511
# define	EIGHT_KB		8192

/*
	TODO:
	It takes too long for uploading files, thinking of incrementing it from 8KB to 500KB or 1MB.
	To see with jawad later: 1048576.
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
		if (conn && conn->isTimedOut())
		{
			std::cout << "Connection timeout for fd " << conn->fd << std::endl;
			if (conn->req)
				conn->req->setState(false, REQUEST_TIMEOUT);
			epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, &ev);
			std::cout << "Closing connection fd " << conn->fd << std::endl;
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
	Connection*					conn;
	ssize_t						bytes;
	std::vector<Connection*>	connections;
	char						buff[EIGHT_KB];
	int							numberOfEvents;
	struct epoll_event			ev, events[MAX_EVENTS];
	time_t						lastTimeoutCheck = time(NULL);

	// ResponseHandler::initialize();

	while (true)
	{
		if (got_singint == true)
			return conn->freeConnections(connections);
		if (time(NULL) - lastTimeoutCheck >= 1)
			checkForTimeouts(connections, ev, epollFd, lastTimeoutCheck);

		// // Cleanup stale connections every 30 seconds
		// if (time(NULL) - lastCleanupCheck >= 30)
		// {
		// 	cleanupStaleConnections(connections, epollFd);
		// 	lastCleanupCheck = time(NULL);
		// }

		numberOfEvents = epoll_wait(epollFd, events, MAX_EVENTS, 1000); //TODO-ACHRAF: check if the 1000 is valid

		for (int i = 0; i < numberOfEvents; i++)
		{
			if (std::find(sockets.begin(), sockets.end(), events[i].data.fd) != sockets.end())
			{
				int clientFd = accept(events[i].data.fd, NULL, NULL);
				if (clientFd == -1)
				{
					std::cout << "Error: failed to accept a client" << std::endl;
					continue;
				}

				Connection* conn = new Connection(clientFd);
				connections.push_back(conn);

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
				{
					bytes = read(conn->fd, buff, EIGHT_KB);
					if (bytes <= 0)
						conn->closeConnection(conn, connections, epollFd);
					else
					{
						if (!conn->req)
						{
							conn->req = new Request(conn->fd);
							conn->cachedLocation = NULL;
						}

						conn->req->appendToBuffer(conn, http, buff, bytes);
						if (!conn->checkMaxBodySize())
							conn->req->setState(false, PAYLOAD_TOO_LARGE);

						if (conn->req->isRequestDone())
						{
							ev.events = EPOLLOUT;
							ev.data.fd = conn->fd;
							epoll_ctl(epollFd, EPOLL_CTL_MOD, conn->fd, &ev);
						}
					}
				} 
				else if (events[i].events & EPOLLOUT)
				{
					if (conn->req)
					{
						try {
							if (conn->fileSendState == 0) {
								conn->res = ResponseHandler::handleRequest(conn);
								std::string responseStr = conn->res.build();
							ssize_t sent = send(conn->fd, responseStr.c_str(), responseStr.size(), 0);
							std::cout<<responseStr<<std::endl;
							std::cout<<"filesendstate: "<<conn->fileSendState<<"contnet-lenght: "<<conn->res.getFileSize()<<std::endl;
								if (sent == -1) {
									handleConnectionError(conn, connections, epollFd, "Header send error");
									conn = NULL;
									continue;
								}
								conn->lastActivityTime = time(NULL);
								std::cout << "headers sent succefuly\n";
								if (!conn->res.getFilePath().empty()) {
									conn->fileFd = open(conn->res.getFilePath().c_str(), O_RDONLY);
									if (conn->fileFd == -1) {
										handleConnectionError(conn, connections, epollFd, "File open error");
										conn = NULL;
										continue;
									}
									conn->fileSendOffset = 0;
									conn->fileSendState = 1;
									break;
									// continue;
								} else {
									conn->fileSendState = 3;
								}
							}
							if (conn->fileSendState == 1) {
							std::cout<<"filesendstate: "<<conn->fileSendState<<std::endl;
								char fileBuf[EIGHT_KB];
								if (lseek(conn->fileFd, conn->fileSendOffset, SEEK_SET) == -1) {
									close(conn->fileFd);
									conn->fileFd = -1;
									handleConnectionError(conn, connections, epollFd, "File seek error");
									conn = NULL;
									continue;
								}
								ssize_t bytesRead = read(conn->fileFd, fileBuf, sizeof(fileBuf));
								if (bytesRead == 0) {
									close(conn->fileFd);
									conn->fileFd = -1;
									conn->fileSendState = 3;
								}
								else if (bytesRead < 0) {
									close(conn->fileFd);
									conn->fileFd = -1;
									handleConnectionError(conn, connections, epollFd, "File send error");
									conn = NULL;
									continue;
								}
								else {
									
									ssize_t bytesSent = send(conn->fd, fileBuf, bytesRead, 0);
									std::cout << "reads  succefuly  "<<bytesSent<<"\n";
									if (bytesSent == -1) {
							std::cout<<"filesendstate: "<<conn->fileSendState<<"contnet-lenght: "<<conn->fileSendOffset<<std::endl;
										conn->fileSendState = 3;
										close(conn->fileFd);
										conn->fileFd = -1;
										break;
									}
									conn->lastActivityTime = time(NULL);
									conn->fileSendOffset += bytesSent;
									if (conn->fileSendOffset >= (ssize_t)conn->res.getFileSize()) {
										close(conn->fileFd);
										conn->fileFd = -1;
										conn->fileSendState = 3;
									}
									break;
								}
							}
							if (conn->fileSendState == 3) {
							std::cout<<"filesendstate: "<<conn->fileSendState<<std::endl;
							std::cout<<"filesendstate: "<<conn->fileSendState<<"contnet-lenght: "<<conn->fileSendOffset<<std::endl;
							
								if (conn->shouldKeepAlive) {
									conn->req->clear();
									conn->fileSendState = 0;
									ev.events = EPOLLIN;
									ev.data.fd = conn->fd;
									epoll_ctl(epollFd, EPOLL_CTL_MOD, conn->fd, &ev);
								} else {
									conn->closeConnection(conn, connections, epollFd);
									conn = NULL;
								}
							}
						} catch (const std::exception& e) {
							std::cout << RED << "Exception in request handling: " << e.what() << RESET << std::endl;
							handleConnectionError(conn, connections, epollFd, "Request handling exception");
							conn = NULL;
							continue;
						}
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
