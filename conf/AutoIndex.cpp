#include "AutoIndex.hpp"

AutoIndex::AutoIndex(void)
{}

AutoIndex::~AutoIndex(void)
{}

DIRTYPE AutoIndex::getType(void) const
{
	return AUTOINDEX;
}

void AutoIndex::setState(bool value)
{
	state = value;
}

bool AutoIndex::getState(void) const {
	return ( state );
}
