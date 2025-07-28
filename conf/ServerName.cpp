#include "ServerName.hpp"

ServerName::ServerName(void)
{
	serverNames = NULL;
}

ServerName::~ServerName(void)
{
	for (unsigned int i = 0; serverNames[i] != NULL; i++)
		free(serverNames[i]);
	delete[] serverNames;
}

DIRTYPE ServerName::getType(void) const
{
	return SERVER_NAME;
}

void ServerName::setServerNames(char** value)
{
	serverNames = value;
}

void ServerName::setServerName(char* value, unsigned int idx)
{
	serverNames[idx] = value;
}
