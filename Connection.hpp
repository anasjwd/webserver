# pragma once

# include "conf/Http.hpp"
# include "conf/Root.hpp"
# include "conf/Server.hpp"
# include "conf/Location.hpp"
# include "conf/AutoIndex.hpp"
# include "conf/ErrorPage.hpp"
# include "conf/IDirective.hpp"
# include "conf/LimitExcept.hpp"
# include "conf/Return.hpp"
# include "request/incs/Request.hpp"
# include "conf/ClientMaxBodySize.hpp"
# include "conf/Index.hpp"
# include "response/include/Response.hpp"

class	Connection
{
	public:
		int				fd;
		Response		res;
		std::string		uri;
		Request			*req;
		bool			connect;
		Server			*conServer;
		bool			shouldKeepAlive;
		time_t			lastTimeoutCheck;
		bool			closed;
		int fileFd;
		int fileSendState; // 0: not started, 1: headers sent, 2: sending body, 3: done
		ssize_t fileSendOffset;


		Connection();
		Connection(int);

		void				updateTime();

		// General ones:
		bool				findServer(Http*);
		IDirective*			getDirective(DIRTYPE type);

		// Alassiqu:
		LimitExcept*		getLimitExcept() const;
		bool				checkMaxBodySize();
		ClientMaxBodySize*	getClientMaxBodySize();

		// ahanaf
		Root*				getRoot();
		Index*				getIndex();
		const Location*		getLocation() const;
		// const Location*		getLocation() const;
		AutoIndex*			getAutoIndex();
		ErrorPage*			getErrorPage();
		
		// 
		Connection*			findConnectionByFd(int, std::vector<Connection*>&);
		void				closeConnection(Connection*, std::vector<Connection*>&, int);
		void				freeConnections(std::vector<Connection*>&);

		Return*				getReturnDirective();
		ErrorPage*			getErrorPageForCode(int);

		std::vector<std::string> _getAllowedMethods() const;
		bool _isAllowedMethod(const std::string& method, const std::vector<std::string>& allowedMethods);



};
