#include <cstdlib>
# include <ctime>
# include <unistd.h>
# include <sys/epoll.h>
# include "conf/Http.hpp"
# include "conf/Root.hpp"
# include "Connection.hpp"
# include "conf/Listen.hpp"
# include "conf/Server.hpp"
# include "conf/Location.hpp"
# include "conf/IDirective.hpp"
# include "conf/ServerName.hpp"
# include "conf/LimitExcept.hpp"
# include "request/incs/Defines.hpp"
# include "conf/ClientMaxBodySize.hpp"

Connection::Connection()
	:	fd(-1), req(NULL), connect(false),
		conServer(NULL), shouldKeepAlive(false), lastTimeoutCheck(time(NULL))
{
}

Connection::Connection(int clientFd)
	:	fd(clientFd), req(NULL), connect(false),
		conServer(NULL), shouldKeepAlive(false), lastTimeoutCheck(time(NULL))
{
}

void	Connection::updateTime()
{
	lastTimeoutCheck = time(NULL);
}

Connection* Connection::findConnectionByFd(int fd, std::vector<Connection*>& connections)
{
	for (std::vector<Connection*>::iterator it = connections.begin(); 
		 it != connections.end(); ++it)
	{
		if ((*it)->fd == fd)
			return *it;
	}
	return NULL;
}

static std::vector<Server*> getServersFromHttp(Http* http)
{
	std::vector<Server*> servers;
	for (size_t i = 0; i < http->directives.size(); ++i) {
		if (http->directives[i]->getType() == SERVER) {
			servers.push_back(static_cast<Server*>(http->directives[i]));
		}
	}
	return servers;
}

bool	Connection::findServer(Http *http)
{
	if (req->getState() < HEADERS || !req->getRequestHeaders().hasHeader("host"))
		return false;

	unsigned int port = req->getRequestHeaders().getHostPort();
	std::string hostname = req->getRequestHeaders().getHostName();
	std::vector<Server*> servers = getServersFromHttp(http);
	for (std::vector<Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
		Server* server = *it;
		for (std::vector<IDirective*>::const_iterator dit = server->directives.begin(); dit != server->directives.end(); ++dit)
		{
			if ((*dit)->getType() == SERVER_NAME)
			{
				ServerName* sn = static_cast<ServerName*>(*dit);
				char** names = sn->getServerNames();
				if (names)
				{
					for (unsigned int i = 0; names[i] != NULL; ++i)
						if (names[i] == hostname)
						{
							conServer = server;
							return true;
						}
				}
			}
			else if ((*dit)->getType() == LISTEN)
			{
				if (static_cast<Listen*>(*dit)->getPort() == port)
				{
					conServer = server;
					return true;
				}
			}
		}
	}
	if (conServer == NULL)
		conServer = servers[0];
	return false;
}

IDirective*	Connection::getDirective(DIRTYPE type)
{
	if (conServer == NULL || conServer->directives.empty())
		return NULL;

	for (std::vector<IDirective*>::iterator dit = conServer->directives.begin(); 
		 dit != conServer->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == type)
			return *dit;
	}
	return NULL;
}

LimitExcept*	Connection::getLimitExcept()
{
	Location*	location = getLocation();
	if (location == NULL)
		return NULL;
	
	for (std::vector<IDirective*>::iterator dit = location->directives.begin(); 
	dit != location->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == LIMIT_EXCEPT)
			return static_cast<LimitExcept*>(*dit);
	}
	return NULL;
}

bool	Connection::checkMaxBodySize()
{
	ClientMaxBodySize* max = getClientMaxBodySize();
	if (max == NULL)
		return true;

	std::string str = req->getRequestHeaders().getHeaderValue("content-length");
	unsigned int actualSize = strtoull(str.c_str(), NULL, 10);

	if (max->getSize() < actualSize)
		return false;

	return true;
}

ClientMaxBodySize*	Connection::getClientMaxBodySize()
{
	return static_cast<ClientMaxBodySize*>(getDirective(CLIENT_MAX_BODY_SIZE));
}

Location*	Connection::getLocation()
{
	return static_cast<Location*>(getDirective(LOCATION));
}

Root*	Connection::getRoot()
{
	Root*	root = static_cast<Root*>(getDirective(ROOT));
	if (root)
		return root;
	Location*	location = getLocation();
	if (location == NULL)
		return NULL;
	
	for (std::vector<IDirective*>::iterator dit = location->directives.begin(); 
	dit != location->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == ROOT)
			return static_cast<Root*>(*dit);
	}
	return NULL;
}

void	Connection::closeConnection(Connection* conn, std::vector<Connection*>& connections, int epollFd)
{
	epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, NULL);

	if (conn->fd != -1)
		close(conn->fd);

	if (conn->req)
		delete conn->req;
	// if (conn.res)
	// 	delete conn.res;

	for (std::vector<Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
	{
		if (*it == conn) {
			connections.erase(it);
			break;
		}
	}

	delete conn;
}
