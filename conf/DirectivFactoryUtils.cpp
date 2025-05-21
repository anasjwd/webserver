#include "cfg_parser.hpp"

bool isValidURL(char* url)
{
	// TODO: implement this
	return ( true );
}

bool isHostAndPort(std::vector<t_token*> tokens, unsigned int pos)
{
	if (strcmp(tokens[pos]->data, "localhost") != 0
			&& strcmp(tokens[pos]->data, "*") != 0
			&& isIpv4(tokens[pos]->data) == false)
		return ( false );
	++pos;
	if (tokens[pos]->type == COLON)
	{
		++pos;
		if (isPortOnly(tokens, pos) == false)
			return ( false );
	}
	else if (tokens[pos]->type != DIR_END)
		return ( false );
	return ( true );
}

bool isPortOnly(std::vector<t_token*> tokens, unsigned int pos)
{
}
