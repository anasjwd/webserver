# pragma once

#include "conf/AutoIndex.hpp"
#include "conf/ClientMaxBodySize.hpp"
#include "conf/ErrorPage.hpp"
#include "conf/LimitExcept.hpp"
#include "conf/Location.hpp"
#include "conf/Root.hpp"
#include "conf/Server.hpp"
#include "request/incs/Request.hpp"
#include "response/include/Response.hpp"

class   Connection
{
	public:
		int			fd;
		Response	*res;
		Request		*req;
		Server*		server;
		bool		connect;
		time_t		lastTimeoutCheck;
		Connection();
	
		Connection*		addConnection(int);
		Connection*		findConnection(int);
		void			eraseConnection(int);

		// Server:
		// alassiqu
		Server*			findServer();
		char**			getLimitExcept();
		unsigned int	getClientMaxBodySize();

		// ahanaf
		Root*			getRoot();
		Location*		getLocation();
		AutoIndex*		getAutoIndex();
		ErrorPage*		getErrorPage();
		


};

/*
	CONNECTION -> CONNECTION HEADER
			: CLOSE ->
			: KEEP-ALIVE -> 
*/

