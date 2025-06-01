#include "ClientMaxBodySize.hpp"

ClientMaxBodySize::ClientMaxBodySize(void)
{}

ClientMaxBodySize::~ClientMaxBodySize(void)
{}

DIRTYPE ClientMaxBodySize::getType(void) const
{
	return CLIENT_MAX_BODY_SIZE;
}

void ClientMaxBodySize::setSize(unsigned int value)
{
	size = value;
}
