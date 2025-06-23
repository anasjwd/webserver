#include "Server.hpp"

Server::Server() : isDefaultServer(false)
{}

Server::~Server(void)
{
}

DIRTYPE Server::getType(void) const
{
	return SERVER;
}

void Server::setIsDefaultServer(bool value)
{
	isDefaultServer = value;
}
