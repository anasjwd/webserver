#include "cfg_parser.hpp"
#define NUM_OF_DIRECTIVES 13

IDirective* createNode(
		DIRTYPE type,
		std::vector<t_token*>& tokens,
		unsigned int& pos)
{
	/*if (pos >= tokens.size())
		// TODO: throw incomplete directive*/ //you send a valid token with valid pos don't you?

	char types[][NUM_OF_DIRECTIVES] = {"server", "listen", "server_name",
		"error_page", "client_max_body_size", "location", "root",
		"limit_except", "return", "index", "autoindex", "deny", "allow"};
	IDirective* (*parsers[])(std::vector<t_token*>&,int&) = {
		parseServerBlock,
		parseListenDirective,
		parseServerNameDirective,
		parseErrorPageDirective,
		parseClientMaxBodySizeDirective,
		parseLocationBlock,
		parseRootDirective,
		parseLimitExceptBlock,
		parseReturnDirective,
		parseIndexDirective,
		parseAutoIndexDirective,
		parseDenyDirective,
		parseAllowDirective,
	};

	for (int i = 0; i < NUM_OF_DIRECTIVES; i++)
	{
		if (strcmp(types[i], tokens[pos]->data) == 0)
		{
			++pos;
			return ( parsers[i](tokens, pos) );
		}
	}
	// TODO: throw: unknown directive
	return ( NULL );
}

void consumeDirectives(
		std::vector<IDirective*>& directives,
		std::vector<t_token*>& tokens,
		unsigned int& pos)
{
	if (pos >= tokens.size())
		// TODO: throw: incomplete directive

	IDirective* holder;
	while (pos < tokens.size() && tokens[pos]->type != BLOCK_END)
	{
		// TODO: try, catch: unkown directive - missing token - allocation failed - creation failed
		// - incomplete directive and throw consumption failed
		holder = createNode(tokens, pos);
		if (holder == NULL)
			return ;
		directives.push_back(holder);
	}
}

IDirective* parseServerBlock(
		std::vector<t_token*>& tokens,
		unsigned int& pos)
{
	if (pos >= tokens.size())
		// TODO: throw incomplete directive
	else if (tokens[pos]->type != BLOCK_START)
		// TODO: throw missing token

	IDirective* server = new Server();

	++pos;
	// TODO: try, catch: consumption failed - incomplete directive, and throw creation fialed
	consumeDirectives(server->directives, pos);
	if (tokens[pos]->type != BLOCK_END)
		//TODO: throw missing token
	else
		++pos;
	// TODO: iterate through the vector and check if there is any server_name
	return ( server );
}

IDirective* parseLocationBlock(
		std::vector<t_token*>& tokens,
		unsigned int& pos)
{
	if (pos >= tokens.size())
		// TODO: throw incomplete directive
	else if (tokens[pos]->type != BLOCK_START)
		// TODO: throw missing token

	IDirective* location = new Location();

	++pos;
	if (tokens[pos]->type == EQUAL)
	{
		location->exactMatch = true;
		++pos;
	}
	if (isValidURL(tokens[pos]) == false)
	{
		std::cerr << "ERROR: Invalid URL" << std::endl;
		delete location;
		return ( NULL );
	}
	location->url = strdup(tokens[pos++]);
	// TODO: try, catch: consumption failed, and throw creation failed
	consumeDirectives(location->directives, pos);
	if (tokens[pos]->type != BLOCK_END)
		//TODO: throw missing token
	else
		++pos;
	return ( location );
}

IDirective* parseListenDirective(
		std::vector<t_token*> tokens,
		unsigned int& pos)
{
	if (pos >= tokens.size())
		// TODO: throw incomplete directive
	if (isPortOnly(tokens, pos)
	{

	}
	else if (isHostAndPort(tokens, pos))
	{

	}
	else
		// TODO: throw: invalid directive content
}
