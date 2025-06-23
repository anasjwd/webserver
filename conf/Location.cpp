#include "Location.hpp"

Location::Location()
{
	uri = NULL;
}

Location::~Location(void)
{
	free(uri);
}

DIRTYPE Location::getType(void) const
{
	return LOCATION;
}

void Location::setUri(char* value)
{
	uri = value;
}

void Location::setExactMatch(bool value)
{
	exactMatch = value;
}
