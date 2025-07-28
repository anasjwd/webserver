#include "Return.hpp"

Return::Return(void)
{
	url = NULL;
}

Return::~Return(void)
{
	if (url) free(url);
}

DIRTYPE Return::getType(void) const
{
	return RETURN;
}

void Return::setCode(unsigned int value)
{
	code = value;
}

void Return::setUrl(char* value)
{
	url = value;
}
