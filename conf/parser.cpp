#include "cfg_parser.hpp"

bool duplicateLocations(BlockDirective* block) {
	std::vector<char*> uris;
	Location* locHolder;
	for (unsigned int i = 0; i < block->directives.size(); i++) {
		if (block->directives[i]->getType() == LOCATION) {
			locHolder = dynamic_cast<Location*>(block->directives[i]);
			uris.push_back(locHolder->getUri());
			if (duplicateLocations(locHolder) == true)
				return ( true );
		}
	}
	std::sort(uris.begin(), uris.end());
	for (unsigned int i = 1; i < uris.size(); i++) {
		if (strcmp(uris[i], uris[i - 1]) == 0)
			return ( true );
	}
	return ( false );
}

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
		std::cerr << "Error: " << e.what() << std::endl;
		delete http;
		return ( NULL );
	}
	if (http->validate() == false)
	{
		delete http;
		return ( NULL );
	}
	for (unsigned int i = 0; i < http->directives.size(); i++) {
		if (http->directives[i]->getType() == SERVER
			&& duplicateLocations(dynamic_cast<Server*>(http->directives[i])) == true) {
			std::cerr << "Error: Duplicate location blocks\n";
			delete http;
			return ( NULL );
		}
	}
	return ( http );
}
