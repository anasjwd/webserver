#include "Server.hpp"

Server::Server()
{}

Server::~Server(void)
{
}

DIRTYPE Server::getType(void) const
{
	return SERVER;
}

bool Server::validate(void)
{
	unsigned int size = directives.size();
	DIRTYPE validTypes[] = {LISTEN, SERVER_NAME, ERROR_PAGE,
		CLIENT_MAX_BODY_SIZE, LOCATION, ROOT, RETURN, INDEX, AUTOINDEX, ALLOW,
		DENY};
	unsigned int j;

	for (unsigned int i = 0; i < size; i++)
	{
		for (j = 0; j < 11; j++)
		{
			if (directives[i]->getType() == validTypes[j])
				break;
		}
		if (j == 11)
		{
			std::cerr << "Error: Invalid directive inside of server block\n";
			return ( false );
		}
	}
	return ( true );
}
