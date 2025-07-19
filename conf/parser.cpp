#include "cfg_parser.hpp"

Http* parser(std::vector<t_token*>& tokens)
{
	Http* http;
	unsigned int pos = 0;

	http = new Http();
	try {
		consumeDirectives(http, tokens, pos, tokens.size());
	}
	catch (std::exception& e)
	{
		delete http;
		std::cerr << "ERROR: " << e.what() << std::endl;
		return ( NULL );
	}
	if (http->validate() == false)
	{
		delete http;
		return ( NULL );
	}
	return http;
}
