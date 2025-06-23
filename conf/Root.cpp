#include "Root.hpp"

Root::Root(void)
{}

Root::~Root(void)
{
	free(path);
}

DIRTYPE Root::getType(void) const
{
	return ROOT;
}

void Root::setPath(char* value)
{
	path = value;
}
