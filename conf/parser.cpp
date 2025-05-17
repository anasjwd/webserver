#include "cfg_parser.hpp"

Directive* parser(std::vector<t_token*> tokens)
{
	Directive* http;

	http = new Http();
	return http;
}
