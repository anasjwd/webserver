#include "Allow.hpp"

Allow::Allow(void)
{}

Allow::~Allow(void)
{
	free(allowed);
}

DIRTYPE Allow::getType(void) const
{
	return ALLOW;
}

void Allow::setAllowed(char* value)
{
	allowed = value;
}

char* Allow::getAllowed(void) const
{
	return ( allowed );
}
