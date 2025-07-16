# pragma once

# include "conf/Http.hpp"
# include "conf/Root.hpp"
# include "conf/Server.hpp"
# include "conf/Location.hpp"
# include "conf/AutoIndex.hpp"
# include "conf/ErrorPage.hpp"
# include "conf/IDirective.hpp"
# include "conf/LimitExcept.hpp"
# include "request/incs/Request.hpp"
# include "conf/ClientMaxBodySize.hpp"
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
		const Location	*matchedLocation;


		Connection();
		Connection(int);

		void				updateTime();

		// General ones:
		bool				findServer(Http*);
		IDirective*			getDirective(DIRTYPE type);

		// Alassiqu:
		LimitExcept*		getLimitExcept();
		bool				checkMaxBodySize();
		ClientMaxBodySize*	getClientMaxBodySize();

		// Ahanaf:
		Root*				getRoot();
		const Location*		getLocation();
		AutoIndex*			getAutoIndex();
		ErrorPage*			getErrorPage();
		
		// 
		void				freeConnections(std::vector<Connection*>&);
		Connection*			findConnectionByFd(int, std::vector<Connection*>&);
		void				closeConnection(Connection*, std::vector<Connection*>&, int);

};
