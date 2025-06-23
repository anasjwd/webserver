#include "Listen.hpp"

Listen::Listen(void)
{
	host = NULL;
}

Listen::~Listen(void)
{
	free(host);
}

DIRTYPE Listen::getType(void) const
{
	return LISTEN;
}

void Listen::setHost(char* value)
{
	host = value;
}

void Listen::setPort(unsigned int value)
{
	port = value;
}

unsigned int Listen::getPort(void) const
{
	return ( port );
}

char* Listen::getHost(void) const
{
	return ( host );
}
