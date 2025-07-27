# include <ctime>
# include <cstddef>
# include <cstdlib>
# include <unistd.h>
# include <algorithm>
# include <sys/epoll.h>
# include "conf/Http.hpp"
# include "conf/Root.hpp"
# include "Connection.hpp"
# include "conf/Listen.hpp"
# include "conf/Server.hpp"
# include "conf/Location.hpp"
# include "conf/AutoIndex.hpp"
# include "conf/IDirective.hpp"
# include "conf/ServerName.hpp"
# include "conf/LimitExcept.hpp"
# include "request/incs/Defines.hpp"
# include "conf/ClientMaxBodySize.hpp"
# include "response/include/ResponseHandler.hpp"

Connection::Connection()
	:	fd(-1), req(NULL), connect(false), conServer(NULL), shouldKeepAlive(false),
		lastActivityTime(time(NULL)), lastTimeoutCheck(time(NULL)),
		closed(false), fileFd(-1), fileSendState(0), fileSendOffset(0),
		cachedLocation(NULL)
{
}

Connection::Connection(int clientFd)
	:	fd(clientFd), req(NULL), connect(false), conServer(NULL), shouldKeepAlive(false),
		lastActivityTime(time(NULL)), lastTimeoutCheck(time(NULL)),
		closed(false), fileFd(-1), fileSendState(0), fileSendOffset(0),
		cachedLocation(NULL)
{
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
	if (!req || req->getState() < HEADERS || !req->getRequestHeaders().hasHeader("host"))
		return false;
	if (req->getState() >= HEADERS && !req->getRequestHeaders().hasHeader("host"))
		return false;

	unsigned int port = req->getRequestHeaders().getHostPort();
	std::string hostname = req->getRequestHeaders().getHostName();
	std::vector<Server*> servers = getServersFromHttp(http);
	
	if (servers.empty())
		return false;
		
	for (std::vector<Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
		Server* server = *it;
		if (!server) continue;
		
		for (std::vector<IDirective*>::const_iterator dit = server->directives.begin(); dit != server->directives.end(); ++dit)
		{
			if (!(*dit)) continue;
			
			if ((*dit)->getType() == SERVER_NAME)
			{
				ServerName* sn = static_cast<ServerName*>(*dit);
				if (sn) {
					char** names = sn->getServerNames();
					if (names)
					{
						for (unsigned int i = 0; names[i] != NULL; ++i)
							if (names[i] == hostname)
							{
								conServer = server;
								cachedLocation = NULL; // Clear cache when server changes
								return true;
							}
					}
				}
			}
			else if ((*dit)->getType() == LISTEN)
			{
				Listen* listen = static_cast<Listen*>(*dit);
				if (listen && listen->getPort() == port)
				{
					conServer = server;
					cachedLocation = NULL; // Clear cache when server changes
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
		if ((*dit) && (*dit)->getType() == type)
			return *dit;
	}
	return NULL;
}

LimitExcept*	Connection::getLimitExcept() const
{
	const Location*	location = getLocation();
	if (location == NULL)
		return NULL;
	
	for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
	dit != location->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == LIMIT_EXCEPT) {
			LimitExcept* limitExcept = static_cast<LimitExcept*>(*dit);
			if (limitExcept) return limitExcept;
		}
	}
	return NULL;
}

bool	Connection::checkMaxBodySize()
{
	unsigned long long maxAllowedSize;
	ClientMaxBodySize* max = getClientMaxBodySize();

	if (max == NULL)
		maxAllowedSize = 1048576;
	else
		maxAllowedSize = max->getSize();

	std::string str = req->getRequestHeaders().getHeaderValue("content-length");
	if (str.empty())
		return true;

	unsigned long long actualSize = strtoull(str.c_str(), NULL, 10);

	if (maxAllowedSize < actualSize)
	{
		std::cout << "MaxBodySize ";
		if (maxAllowedSize == 1048576)
			std::cout << "default (1M) detected!\n";
		else
			std::cout << "from config file detected!\n";
		return false;
	}
	return true;
}

ClientMaxBodySize*	Connection::getClientMaxBodySize()
{
	ClientMaxBodySize* maxSize = static_cast<ClientMaxBodySize*>(getDirective(CLIENT_MAX_BODY_SIZE));
	if (maxSize) return maxSize;
	
	// Check location context if not found in server
	const Location* location = getLocation();
	if (location) {
		for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
		dit != location->directives.end(); ++dit) 
		{
			if ((*dit)->getType() == CLIENT_MAX_BODY_SIZE) {
				ClientMaxBodySize* locMaxSize = static_cast<ClientMaxBodySize*>(*dit);
				if (locMaxSize) return locMaxSize;
			}
		}
	}
	return NULL;
}

const Location* Connection::getLocation() const
{
	// Return cached location if available
	if (cachedLocation)
	{
		std::cout << "[CACHE HIT] Using cached location for URI: " << (req ? req->getRequestLine().getUri() : uri) << std::endl;
		return cachedLocation;
	}

	if (!conServer)
		return NULL;

	std::string reqUri;
	if (req && req->getRequestLine().getUri().size())
		reqUri = req->getRequestLine().getUri();
	else if (!uri.empty())
		reqUri = uri;
	else
		return NULL;

	std::cout << "[CACHE MISS] Computing location for URI: " << reqUri << std::endl;
	// Compute location once and cache it
	cachedLocation = _findBestLocation(conServer->directives, reqUri);
	return cachedLocation;
}

const Location* Connection::_findBestLocation(const std::vector<IDirective*>& directives, const std::string& reqUri) const
{
	const Location* longestPrefixMatch = NULL;
	size_t bestMatchLen = 0;
	const Location* cgiMatch = NULL;

	std::string cgiExtension;
	size_t dotPos = reqUri.rfind('.');
	if (dotPos != std::string::npos) {
		size_t slashAfterDot = reqUri.find('/', dotPos);
		if (slashAfterDot == std::string::npos || slashAfterDot > dotPos) {
			cgiExtension = reqUri.substr(dotPos);
		}
	}

	std::cout << "[DEBUG] reqUri: '" << reqUri << "' cgiExtension: '" << cgiExtension << "'" << std::endl;

	for (std::vector<IDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it) {
		if ((*it)->getType() != LOCATION)
			continue;
		const Location* loc = static_cast<const Location*>(*it);
		if (!loc) continue;

		std::string locUri = loc->getUri();
		bool exact = loc->isExactMatch();

		std::cout << "[DEBUG] Checking location: '" << locUri << "' exact: " << exact << std::endl;

		if (exact) {
			if (reqUri == locUri) {
				std::cout << "[DEBUG] Exact match found: " << locUri << std::endl;
				return loc;
			}
		} else {
			if (!locUri.empty() && locUri[0] == '.') {
				if (!cgiExtension.empty() && locUri == cgiExtension) {
					std::cout << "[DEBUG] CGI match found: " << locUri << std::endl;
					cgiMatch = loc;
				}
			} else {
				// For prefix matching, check if request URI starts with location URI
				bool matches = false;
				if (locUri == "/") {
					// Special case: if location is "/", any URI starting with "/" should match
					matches = (reqUri[0] == '/');
				} else {
					// Check if request URI starts with location URI
					// For /about/somt to match /about/, we need to check if it starts with /about/
					if (reqUri.compare(0, locUri.length(), locUri) == 0) {
						matches = true;
					}
				}
				
				if (matches) {
					if (locUri.length() > bestMatchLen) {
						bestMatchLen = locUri.length();
						longestPrefixMatch = loc;
						std::cout << "[DEBUG] New longest prefix match: " << locUri << " (length: " << locUri.length() << ")" << std::endl;
					}
				}
			}
		}
	}

	if (cgiMatch) {
		std::cout << "[DEBUG] Returning CGI match: " << cgiMatch->getUri() << std::endl;
		return cgiMatch;
	}

	if (longestPrefixMatch) {
		std::cout << "[DEBUG] Returning longest prefix match: " << longestPrefixMatch->getUri() << std::endl;
		const Location* nestedMatch = _findBestLocation(longestPrefixMatch->directives, reqUri);
		if (nestedMatch) {
			return nestedMatch;
		}
		return longestPrefixMatch;
	}

	std::cout << "[DEBUG] No match found" << std::endl;
	return NULL;
}


/* 
const Location* Connection::getLocation() const
{
	if (!conServer)
		return NULL;
	std::string reqUri;
	if (req && req->getRequestLine().getUri().size())
		reqUri = req->getRequestLine().getUri();
	else if (!uri.empty())
		reqUri = uri;
	else
		return NULL;

	const Location* bestLoc = NULL;
	size_t bestMatchLen = 0;
	for (std::vector<IDirective*>::const_iterator it = conServer->directives.begin(); it != conServer->directives.end(); ++it) {
		if ((*it)->getType() != LOCATION)
			continue;
		const Location* loc = static_cast<const Location*>(*it);
		if (!loc) continue;
		char* locUri = loc->getUri();
		bool exact = loc->isExactMatch();
		if (!locUri) continue;
		std::string locUriStr(locUri);
		std::cout << "[getLocation] reqUri: '" << reqUri << "' locUri: '" << locUriStr << "' exact: " << exact << std::endl;
		if (exact) {
			if (reqUri == locUriStr) {
				std::cout << "[getLocation] Exact match found!" << std::endl;
				return loc;
			}
		} else {
			if (!locUriStr.empty() &&
			    (reqUri == locUriStr ||
			     (reqUri.find(locUriStr + "/") == 0)) &&
			    locUriStr.length() > bestMatchLen) {
				bestMatchLen = locUriStr.length();
				bestLoc = loc;
			}
		}
	}
	if (bestLoc) {
		std::cout << BLUE << bestLoc->getUri() << RESET << std::endl;
		std::cout << "[getLocation] Prefix match found!" << std::endl;
		return bestLoc;
	}
	std::cout << "[getLocation] No match found." << std::endl;
	return NULL;
}
 */
Root*	Connection::getRoot()
{
	Root*	root = static_cast<Root*>(getDirective(ROOT));
	if (root)
		return root;
	const Location*	location = getLocation();
	if (location == NULL)
		return NULL;
	
	for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
	dit != location->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == ROOT) {
			Root* locRoot = static_cast<Root*>(*dit);
			if (locRoot) return locRoot;
		}
	}
	return NULL;
}

AutoIndex* Connection::getAutoIndex()
{
	const Location* location = getLocation();
	if (location) {
		char* locUri = location->getUri();
		if (locUri) std::cout << locUri << std::endl;
		for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
		dit != location->directives.end(); ++dit) 
		{
			if ((*dit)->getType() == AUTOINDEX) {
				AutoIndex* locAutoIndex = static_cast<AutoIndex*>(*dit);
				if (locAutoIndex) {
					return locAutoIndex;
				}
			}
		}
	}
	AutoIndex* autoindex = static_cast<AutoIndex*>(getDirective(AUTOINDEX));
	if (autoindex) {
		return autoindex;
	}
	static AutoIndex defaultOff;
	defaultOff.setState(false);
	return &defaultOff;
}

Return* Connection::getReturnDirective(){
	const Location* location = getLocation();
	if (location) {
		IDirective* dir = location->getDirective(RETURN);
		if (dir) {
			Return* ret = static_cast<Return*>(dir);
			if (ret) return ret;
		}
	}
	if (conServer) {
		for (std::vector<IDirective*>::const_iterator dit = conServer->directives.begin(); dit != conServer->directives.end(); ++dit) {
			if ((*dit)->getType() == RETURN) {
				Return* ret = static_cast<Return*>(*dit);
				if (ret) return ret;
			}
		}
	}
	return NULL;
}

Index* Connection::getIndex() {
	const Location* location = getLocation();
	if (location) {
		for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
		dit != location->directives.end(); ++dit) 
		{
			if ((*dit)->getType() == INDEX) {
				return static_cast<Index*>(*dit);
			}
		}
	}
	Index* index = static_cast<Index*>(getDirective(INDEX));
	if (index)
		return index;
	return NULL;
}

ErrorPage* Connection::getErrorPage() {
	ErrorPage* errorPage = static_cast<ErrorPage*>(getDirective(ERROR_PAGE));
	if (errorPage)
		return errorPage;
	
	const Location* location = getLocation();
	if (location) {
		for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
		dit != location->directives.end(); ++dit) 
		{
			if ((*dit)->getType() == ERROR_PAGE) {
				ErrorPage* locErrorPage = static_cast<ErrorPage*>(*dit);
				if (locErrorPage) return locErrorPage;
			}
		}
	}
	return NULL;
}

ErrorPage* Connection::getErrorPageForCode(int code) {
	const Location* location = getLocation();
	if (location) {
		for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); dit != location->directives.end(); ++dit) {
			if ((*dit)->getType() == ERROR_PAGE) {
				ErrorPage* ep = static_cast<ErrorPage*>(*dit);
				if (ep && (ep->getCode() == code || ep->getResponseCode() == code))
					return ep;
			}
		}
	}
	if (conServer) {
		for (std::vector<IDirective*>::const_iterator dit = conServer->directives.begin(); dit != conServer->directives.end(); ++dit) {
			if ((*dit)->getType() == ERROR_PAGE) {
				ErrorPage* ep = static_cast<ErrorPage*>(*dit);
				if (ep && (ep->getCode() == code || ep->getResponseCode() == code))
					return ep;
			}
		}
	}
	return NULL;
}

void Connection::closeConnection(Connection* conn, std::vector<Connection*>& connections, int epollFd)
{
	if (!conn || conn->closed)
		return;

	conn->closed = true;

	if (epollFd != -1 && conn->fd != -1)
		epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, NULL);

	if (conn->fileFd != -1)
	{
		close(conn->fileFd);
		conn->fileFd = -1;
	}
	if (conn->fd != -1)
	{
		close(conn->fd);
		conn->fd = -1;
	}

	delete conn->req;
	conn->req = NULL;

	connections.erase(
		std::remove(connections.begin(), connections.end(), conn),
		connections.end()
	);
	if (std::find(connections.begin(), connections.end(), conn) != connections.end())
	{
		std::cout << "Failed to remove connection from vector" << std::endl;
	}
	else
	{
		std::cout << "Connection successfully removed from vector" << std::endl;
		std::cout << "Remaining connections: " << connections.size() << std::endl;
		if (conn->req == NULL)
			std::cout << "Request pointer is NULL" << std::endl;
		else
			std::cout << "Request pointer is not NULL" << std::endl;
	}

	delete conn;
	// conn = NULL;
	std::cout << "Connection closed and deleted" << std::endl;
}

void	Connection::freeConnections(std::vector<Connection*>& connections)
{
    for (std::vector<Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        Connection* conn = *it;
        if (conn->req)
        {
            delete conn->req;
            conn->req = NULL;
        }
        if (conn->fileFd != -1)
        {
            close(conn->fileFd);
            conn->fileFd = -1;
        }
        if (conn->fd != -1)
        {
            close(conn->fd);
            conn->fd = -1;
        }
        delete conn;
    }
    connections.clear();
}


std::vector<std::string> Connection::_getAllowedMethods() const {
    std::vector<std::string> methods;
    LimitExcept* limitExcept = getLimitExcept();
    if (limitExcept) {
        char** allowedMethods = limitExcept->getMethods();
        if (allowedMethods) {
            for (int i = 0; allowedMethods[i] != NULL; ++i) {
                methods.push_back(std::string(allowedMethods[i]));
            }
        }
    }
    if (methods.empty()) {
        methods.push_back("GET");
        methods.push_back("POST");
        methods.push_back("DELETE");
    }
    return methods;
}

bool Connection::_isAllowedMethod(const std::string& method, const std::vector<std::string>& allowedMethods) {
    if (method.empty()) return false;
    return std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
}

bool	Connection::isTimedOut() const
{
	return time(NULL) - lastActivityTime > TIMEOUT_SECONDS;
}
