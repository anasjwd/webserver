#include "Http.hpp"

Http::Http(void)
{
}

Http::~Http(void)
{}

DIRTYPE Http::getType(void) const
{
	return HTTP;
}

bool Http::validate(void)
{
	unsigned int size = directives.size();
	DIRTYPE validTypes[] = {SERVER, ERROR_PAGE, CLIENT_MAX_BODY_SIZE, ROOT,
		INDEX, AUTOINDEX, DENY, ALLOW};
	unsigned int j;

	for (unsigned int i = 0; i < size; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (directives[i]->getType() == validTypes[j])
				break;
		}
		if (j == 8)
		{
			return ( false );
		}
	}
	return ( true );
	/*for (unsigned int i = 0; i < size; i++) {
		if (directives[i]->getType() == SERVER) {
			return ( true );
		}
	}
	return ( false );*/
}
