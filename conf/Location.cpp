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

bool Location::validate(void)
{
	unsigned int size = directives.size();
	DIRTYPE validTypes[] = {ERROR_PAGE, CLIENT_MAX_BODY_SIZE, ROOT,
		LIMIT_EXCEPT, RETURN, INDEX, AUTOINDEX, DENY, ALLOW};
	unsigned int j;

	for (unsigned int i = 0; i < size; i++)
	{
		for (j = 0; j < 9; j++)
		{
			if (directives[i]->getType() == validTypes[j])
				break;
		}
		if (j == 9)
		{
			std::cerr << "Error: Invalid directive inside of location block\n";
			return ( false );
		}
	}
	return ( true );
}
