#include "DirectiveFactory.hpp"
#define NUM_OF_DIRECTIVES 13

IDirective* DirectiveFactory::create(
		DIRTYPE type,
		std::vector<t_token*>& tokens,
		int& pos)
{
	if (pos >= tokens.size())
		//TODO: throw exception
		return NULL;

	char* types[NUM_OF_DIRECTIVES] = {"server", "listen", "server_name", "error_page",
	"client_max_body_size", "location", "root", "limit_except", "return",
	"index", "autoindex", "deny", "allow"};
	IDirective* (*parsers[])(std::vector<t_token*>&,int&) = {
		&DirectiveFactory::parseServerBlock,
		&DirectiveFactory::parseListenDirective,
		&DirectiveFactory::parseServerNameDirective,
		&DirectiveFactory::parseErrorPageDirective,
		&DirectiveFactory::parseClientMaxBodySizeDirective,
		&DirectiveFactory::parseLocationBlock,
		&DirectiveFactory::parseRootDirective,
		&DirectiveFactory::parseLimitExceptBlock,
		&DirectiveFactory::parseReturnDirective,
		&DirectiveFactory::parseIndexDirective,
		&DirectiveFactory::parseAutoIndexDirective,
		&DirectiveFactory::parseDenyDirective,
		&DirectiveFactory::parseAllowDirective,
	};

	for (int i = 0; i < NUM_OF_DIRECTIVES; i++)
	{
		if (strcmp(types[i], tokens[pos]->data) == 0)
			return ( parsers[i](tokens, pos) );
	}
	// TODO: throw exception
	return ( NULL );
}

void DirectiveFactory::parseBlock(
		std::vector<IDirective*>& directives,
		std::vector<t_token*>& tokens,
		int& pos)
{
	if (pos >= tokens.size())
		// TODO: throw exception
		return ;
	while (pos < tokens.size() && tokens[pos].type != BLOCK_END)
		directives.push_back(DirectiveFactory::create(tokens, pos));
}

IDirective* parseServerBlock(std::vector<t_token*>& tokens, int& pos)
{
}
