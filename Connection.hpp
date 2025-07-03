# pragma once

#include "request/incs/Request.hpp"
#include "response/include/Response.hpp"

class   Connection
{
	private:
		int			fd;
		Request		*req;
		Response	*res;
		bool		connect;
		time_t		lastTimeoutCheck;

	public:
		Connection()
			:	fd(-1), req(NULL), res(NULL), connect(false)
		{
		}


};

/*
	CONNECTION -> CONNECTION HEADER
			: CLOSE ->
			: KEEP-ALIVE -> 
*/