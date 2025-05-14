#include "cfg_parser.hpp"

Directive* parser(std::vector<t_token*> tokens)
{
	Directive* http;

	(void)tokens;
	http = new Http();
	return http;
}
