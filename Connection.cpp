# include <ctime>
# include <csignal>
# include <cstddef>
# include <cstdlib>
# include <unistd.h>
# include <algorithm>
# include <sys/wait.h>
# include <sys/epoll.h>
# include "conf/Http.hpp"
# include "conf/Root.hpp"
# include "Connection.hpp"
# include "conf/Listen.hpp"
# include "conf/Server.hpp"
# include "conf/Upload.hpp"
# include "conf/Location.hpp"
# include "conf/AutoIndex.hpp"
# include "conf/IDirective.hpp"
# include "conf/ServerName.hpp"
# include "conf/LimitExcept.hpp"
# include "conf/UploadLocation.hpp"
# include "request/incs/Defines.hpp"
# include "conf/ClientMaxBodySize.hpp"
#include "response/include/Response.hpp"
# include "response/include/ResponseHandler.hpp"

Connection::Connection()
    :   fd(-1), req(NULL), connect(false), conServer(NULL),
        lastActivityTime(time(NULL)), lastTimeoutCheck(time(NULL)),
        uploadAuthorized(false),
		closed(false), fileFd(-1), fileSendState(0), fileSendOffset(0), 
		headersSent(false),
        isCgi(false), cgiExecuted(false), cgiCompleted(false), cgiPid(-1), cgiReadState(0),
        cgiStartTime(0), cachedLocation(NULL)
{
	std::cout << BG_BLUE << "Connection default constructor called "<< RESET << std::endl;
    cgiPipeFromChild[0] = -1;
    cgiPipeFromChild[1] = -1;
    cgiPipeToChild[0] = -1;
    cgiPipeToChild[1] = -1;
}

Connection::Connection(int clientFd)
    :   fd(clientFd), req(NULL), connect(false), conServer(NULL),
        lastActivityTime(time(NULL)), lastTimeoutCheck(time(NULL)),
        closed(false), fileFd(-1), fileSendState(0), fileSendOffset(0), headersSent(false),
        isCgi(false), cgiExecuted(false), cgiCompleted(false), cgiPid(-1), cgiReadState(0),
        cgiStartTime(0), cachedLocation(NULL)
{
	std::cout << BG_BLUE << "Connection param constructor called for fd " << fd << RESET << std::endl;
    cgiPipeFromChild[0] = -1;
    cgiPipeFromChild[1] = -1;
    cgiPipeToChild[0] = -1;
    cgiPipeToChild[1] = -1;
}

Connection::~Connection()
{
	std::cout << BG_BLUE << "Connection destructor called for fd " << fd << RESET << std::endl;
}

void Connection::resetCgiState() {
    std::cout << "[DEBUG] Resetting CGI state for fd " << fd << std::endl;
    
    if (cgiPid > 0) {
        std::cout << "[DEBUG] Killing existing CGI process " << cgiPid << std::endl;
        kill(cgiPid, SIGTERM);
        usleep(10000); // Wait 10ms
        kill(cgiPid, SIGKILL);
        waitpid(cgiPid, NULL, WNOHANG); // Clean up zombie
    }
    
    isCgi = false;
    cgiExecuted = false;
    cgiCompleted = false;
    cgiPid = -1;
    cgiReadState = 0;
    cgiStartTime = 0;
    
    if (cgiPipeFromChild[0] != -1) {
        close(cgiPipeFromChild[0]);
        cgiPipeFromChild[0] = -1;
    }
    if (cgiPipeFromChild[1] != -1) {
        close(cgiPipeFromChild[1]);
        cgiPipeFromChild[1] = -1;
    }
    if (cgiPipeToChild[0] != -1) {
        close(cgiPipeToChild[0]);
        cgiPipeToChild[0] = -1;
    }
    if (cgiPipeToChild[1] != -1) {
        close(cgiPipeToChild[1]);
        cgiPipeToChild[1] = -1;
    }
    
    cgiHeaders.clear();
    cgiBody.clear();
    cgiOutput.clear();
    
    cgiResponse.clear();
    
    std::cout << "[DEBUG] CGI state reset complete for fd " << fd << std::endl;
}

Connection* Connection::findConnectionByFd(int fd, std::vector<Connection*>& connections)
{
	if (connections.size() == 0)
		return NULL;
	std::cout << "Size of connections: " << connections.size() << "\n";
	for (std::vector<Connection*>::iterator it = connections.begin(); 
		it != connections.end(); ++it)
	{
		std::cout << "test\n";
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
	std::cout << "1\n";
	std::cout << req->getState() << " " << HEADERS << std::endl;
	std::cout << req->getRequestHeaders().hasHeader("host") << std::endl;

	if (!req || req->getState() < HEADERS || !req->getRequestHeaders().hasHeader("host"))
		return false;
	std::cout << "2\n";

	if (req->getState() >= HEADERS && !req->getRequestHeaders().hasHeader("host"))
		return false;
	std::cout << "3\n";

	unsigned int port = req->getRequestHeaders().getHostPort();
	std::string hostname = req->getRequestHeaders().getHostName();
	std::vector<Server*> servers = getServersFromHttp(http);
	std::cout << "4\n";
	
	if (servers.empty())
		return false;
	std::cout << "5\n";
		
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
								cachedLocation = NULL;
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
					cachedLocation = NULL; 
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

void	Connection::getUpload()
{
	std::cout << "About to get Location\n";
	const Location*	location = getLocation();
	if (location == NULL)
		return ;

	std::cout << "Location:" << location->getUri() << "\n";
	for (std::vector<IDirective*>::const_iterator dit = location->directives.begin(); 
	dit != location->directives.end(); ++dit) 
	{
		if ((*dit)->getType() == UPLOAD)
			uploadAuthorized = static_cast<Upload*>(*dit)->getState();
		else if ((*dit)->getType() == UPLOAD_LOCATION)
			uploadLocation = static_cast<UploadLocation*>(*dit)->getLocation();
	}
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

std::string  Connection::_normalizeUri(const std::string& uri) {
    std::string result;
    bool prevSlash = false;
    
    for (size_t i = 0; i < uri.size(); ++i) {
        if (uri[i] == '/') {
            if (!prevSlash) {
                result += '/';
            }
            prevSlash = true;
        } else {
            result += uri[i];
            prevSlash = false;
        }
    }
    return result.empty() ? "/" : result;
}



const Location* Connection::getLocation() const
{
	std::cout << "In location\n";
	if (cachedLocation)
	{
		std::cout << "[CACHE HIT] Using cached location for URI: " << (req ? req->getRequestLine().getUri() : uri) << std::endl;
		return cachedLocation;
	}

	if (!conServer)
	{
		std::cout << "ConServer NULL\n";
		return NULL;
	}

	std::string reqUri;
	if (req && req->getRequestLine().getUri().size() > 0)
		reqUri = req->getRequestLine().getUri();
	else if (!uri.empty())
		reqUri = uri;
	else
	{
		std::cout << "NULL case!\n";
		return NULL;
	}
	reqUri = _normalizeUri(reqUri);
	cachedLocation = _findBestLocation(conServer->directives, reqUri);
	if (cachedLocation == NULL)
		std::cout << "_findBestLocation is NULL\n";
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

	for (std::vector<IDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it) {
		if ((*it)->getType() != LOCATION)
			continue;
		const Location* loc = static_cast<const Location*>(*it);
		if (!loc) continue;

		std::string locUri = loc->getUri();
		if (!locUri.empty() && locUri[0] == '.') {
			if (!cgiExtension.empty() && locUri == cgiExtension) {
				cgiMatch = loc;
			}
		} else {
			// For prefix matching, check if request URI starts with location URI
			bool matches = false;
			if (locUri == "/") {
				// Special case: if location is "/", any URI starting with "/" should match
				matches = (reqUri[0] == '/');
			} else {
				// For /about/somt to match /about/, we need to check if it starts with /about/
				if (reqUri.compare(0, locUri.length(), locUri) == 0) {
					matches = true;
				}
			}
			
			if (matches) {
				if (locUri.length() > bestMatchLen) {
					bestMatchLen = locUri.length();
					longestPrefixMatch = loc;
				}
			}
		}
		
	}

	if (cgiMatch) {
		return cgiMatch;
	}

	if (longestPrefixMatch) {
		const Location* nestedMatch = _findBestLocation(longestPrefixMatch->directives, reqUri);
		if (nestedMatch) {
			return nestedMatch;
		}
		return longestPrefixMatch;
	}
	return NULL;
}

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

	// connections.erase(
	// 	std::remove(connections.begin(), connections.end(), conn),
	// 	connections.end()
	// );
	connections.erase(find(connections.begin(), connections.end(), conn));
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
	std::cout << ">>>>>>>>>>>>>> size = " << connections.size() << std::endl;
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

bool	Connection::isCgiTimedOut() const
{
	std::cout << BG_CYAN << "Checking CGI timeout for fd " << fd << std::endl;
	std::cout << "CGI time value: " << time(NULL) - cgiStartTime << RESET << std::endl;
	return time(NULL) - cgiStartTime >= CGI_TIMEOUT;
}

bool	Connection::isTimedOut() const
{
	return time(NULL) - lastActivityTime >= TIMEOUT_SECONDS;
}

void	Connection::epollinProcess(Http* http, Connection* conn, std::vector<Connection*>& connections, struct epoll_event& ev, int epollFd)
{
	ssize_t	bytes;
	char	buff[1048576];

	std::cout << RED << "EPOLLIN\n" << RESET;
	bytes = read(fd, buff, 1048576);
	if (bytes <= 0)
		conn->closeConnection(conn, connections, epollFd);
	else
	{
		if (!req)
		{
			req = new Request(fd);
			cachedLocation = NULL;
			resetCgiState();
		}
		req->appendToBuffer(this, http, buff, bytes);
		if (!checkMaxBodySize())
			req->setState(false, PAYLOAD_TOO_LARGE);
		if (req->isRequestDone())
		{
			std::cout << RED << "Request done with state:" << req->getStatusCode() << RESET << "\n";
			ev.events = EPOLLOUT;
			ev.data.fd = fd;
			epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
		}
	}
}
