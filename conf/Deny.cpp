#include "Deny.hpp"

Deny::Deny(void)
{}

Deny::~Deny(void)
{
	free(denied);
}

DIRTYPE Deny::getType(void) const
{
	return DENY;
}

void Deny::setDenied(char* value)
{
	denied = value;
}

char* Deny::getDenied(void) const
{
	return ( denied );
}
