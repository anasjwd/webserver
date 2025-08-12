#include "cfg_parser.hpp"
#define NUM_OF_DIRECTIVES 11

IDirective* createNode(std::vector<t_token*>&,unsigned int&);
void consumeDirectives(
		BlockDirective* block,
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize,
		TOKEN end);

IDirective* parseServerBlock(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type != BLOCK_START)
		throw DirectiveException("incomplete server block - missing \"{\"");

	Server* server = new Server();
	++pos;
	if (pos >= tokensSize)
	{
		delete server;
		throw DirectiveException("incomplete server block - missing \"}\"");
	}
	try {
		consumeDirectives(server, tokens, pos, tokensSize, BLOCK_END);
	}
	catch (std::exception& e)
	{
		delete server;
		throw;
	}
	if (pos >= tokensSize || tokens[pos]->type != BLOCK_END)
	{
		delete server;
		throw DirectiveException("incomplete server block - missing \"}\"");
	}
	else
		++pos;
	if (server->validate() == false)
	{
		delete server;
		throw DirectiveException("invalid content inside of server block");
	}
	return ( server );
}

IDirective* parseLocationBlock(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize)
		throw DirectiveException("incomplete location block - missing URI");

	Location* location = new Location();
	if (tokens[pos]->type == EQUAL)
	{
		location->setExactMatch(true);
		++pos;
		if (pos >= tokensSize)
		{
			delete location;
			throw DirectiveException("incomplete location block - missing URI");
		}
	}
	if (isValidURI(tokens[pos]->data) == false || tokens[pos]->type != STRING)
	{
		delete location;
		throw DirectiveException("invalid listen's content - invalid URI");
	}
	location->setUri(strdup(tokens[pos++]->data));
	if (pos >= tokensSize || tokens[pos]->type != BLOCK_START)
	{
		delete location;
		throw DirectiveException("incomplete location block - missing \"{\"");
	}
	++pos;
	try {
		consumeDirectives(location, tokens, pos, tokensSize, BLOCK_END);
	}
	catch (std::exception& e)
	{
		delete location;
		throw;
	}
	if (pos >= tokensSize || tokens[pos]->type != BLOCK_END)
	{
		delete location;
		throw DirectiveException("incomplete location block - missing \"}\"");
	}
	else
		++pos;
	if (location->validate() == false)
	{
		delete location;
		throw DirectiveException("invalid directive inside of location block");
	}
	return ( location );
}

IDirective* parseListenDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize)
		throw DirectiveException("incomplete listen directive - missing host/port");

	Listen *listen = new Listen();
	if (isPortOnly(tokens, pos))
	{
		listen->setHost(strdup("0.0.0.0"));
		std::stringstream ss(tokens[pos++]->data);
		unsigned int port;
		ss >> port;
		listen->setPort(port);
	}
	else if (isHostAndPort(tokens, pos, tokensSize))
	{
		listen->setHost(strdup(tokens[pos++]->data));
		if (pos >= tokensSize)
		{
			delete listen;
			throw DirectiveException("incomplete listen directive - missing \";\"");
		}
		if (tokens[pos]->type == COLON)
		{
			++pos;
			if (pos >= tokensSize)
			{
				delete listen;
				throw DirectiveException("incomplete listen directive - missing port");
			}
			std::stringstream ss(tokens[pos++]->data);
			unsigned int port;
			ss >> port;
			listen->setPort(port);
		}
		else
			listen->setPort(8080);
	}
	else
	{
		delete listen;
		throw DirectiveException("invalid content for listen directive");
	}
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete listen;
		throw DirectiveException("incomplete listen directive - missin \";\"");
	}
	else
		pos++;
	return ( listen );
}

bool isDirective(char* str)
{
	char dirs[][21] = {"server", "listen", "server_name", "error_page",
		"client_max_body_size", "location", "root", "limit_except",
		"return", "index", "autoindex"};

	for (unsigned int i = 0; i < NUM_OF_DIRECTIVES; i++)
	{
		if (strcmp(str, dirs[i]) == 0)
			return ( true );
	}
	return ( false );
}

IDirective* parseServerName(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize)
		throw DirectiveException("incomplete server_name directive - missing names");

	ServerName* serverName = new ServerName();
	unsigned int idx = pos;
	unsigned int size;
	while (idx < tokensSize && tokens[idx]->type != DIR_END)
		idx++;
	if (idx >= tokensSize)
	{
		delete serverName;
		throw DirectiveException("incomplete server_name directive - missing \";\"");
	}
	size = idx - pos + 1;
	serverName->setServerNames(new char*[size + 1]);
	idx = 0;
	while (idx <= size)
		serverName->setServerName(NULL, idx++);
	idx = 0;
	while (tokens[pos]->type != DIR_END)
	{
		if (isDirective(tokens[pos]->data) == true)
		{
			delete serverName;
			throw DirectiveException("incomplete server_name directive - missing \";\"");
		}
		serverName->setServerName(strdup(tokens[pos]->data), idx);
		++pos;
		++idx;
	}
	++pos;
	return ( serverName );
}

IDirective* parseLimitExceptDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize)
		throw DirectiveException("incomplete limit_except block - missing methods");

	LimitExcept* limitExcept = new LimitExcept();
	unsigned int idx = pos;
	while (idx < tokensSize && tokens[idx]->type != DIR_END)
	{
		if (isValidMethod(tokens[idx]->data) == false)
		{
			delete limitExcept;
			throw DirectiveException("invalid content for limit_except directive - invalid method");
		}
		idx++;
	}
	if (idx >= tokensSize)
	{
		delete limitExcept;
		throw DirectiveException("incomplete limit_except directive - missing \";\"");
	}
	else if (idx == 0)
	{
		delete limitExcept;
		throw DirectiveException("incomplete limit_except directive - missing argument");
	}
	unsigned int size = idx - pos + 1;
	limitExcept->setMethods(new char*[size + 1]);
	idx = 0;
	while (idx <= size)
		limitExcept->setMethod(NULL, idx++);
	idx = 0;
	while (pos < tokensSize && tokens[pos]->type != DIR_END)
	{
		limitExcept->setMethod(strdup(tokens[pos]->data), idx);
		++pos;
		++idx;
	}
	++pos;
	return ( limitExcept );
}

IDirective* parseRootDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type != STRING)
		throw DirectiveException("incomplete root directive - missing path");

	Root* root = new Root();
	root->setPath(strdup(tokens[pos++]->data));
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete root;
		throw DirectiveException("incomplete root directive - missing \";\"");
	}
	pos++;
	return ( root );
}

IDirective* parseErrorPage(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize)
		throw DirectiveException("incomplete error_page directive - missing code");

	ErrorPage* errorPage = new ErrorPage();
	std::stringstream ss(tokens[pos++]->data);
	int tmp;
	ss >> tmp;
	errorPage->setCode(tmp);
	if (ss.fail() || ss.eof() == false)
	{
		delete errorPage;
		throw DirectiveException("invalid content for error_page directive - invalid code number");
	}
	if (tokens[pos]->type == DIR_END)
	{
		delete errorPage;
		throw DirectiveException("invalid number of arguments for error_page directive");
	}
	if (errorPage->getCode() < 300 || errorPage->getCode() > 599)
	{
		delete errorPage;
		throw DirectiveException("invalid content for error_page directive - code must be between 300 and 599");
	}
	// Accept only <uri> as the next argument
	if (tokens[pos]->type != STRING) {
		delete errorPage;
		throw DirectiveException("invalid content for error_page directive - missing or invalid uri");
	}
	errorPage->setUri(strdup(tokens[pos++]->data));
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete errorPage;
		throw DirectiveException("incomplete error_page directive - missing ';'");
	}
	else
		++pos;
	return ( errorPage );
}

IDirective* parseIndexDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("incomplete index directive - missing file name");

	Index* index = new Index();
	unsigned int idx = pos;
	while (idx < tokensSize && tokens[idx]->type != DIR_END)
		idx++;
	if (idx >= tokensSize)
	{
		delete index;
		throw DirectiveException("incomplete index directive - missing \";\"");
	}
	unsigned int size = idx - pos;
	index->setFiles(new char*[size + 1]);
	idx = 0;
	while (idx <= size)
		index->setFile(NULL, idx++);
	idx = 0;
	while (tokens[pos]->type != DIR_END)
	{
		if (isDirective(tokens[pos]->data) == true)
		{
			delete index;
			throw DirectiveException("incomplete index directive - missing \";\"");
		}
		index->setFile(strdup(tokens[pos]->data), idx);
		++pos;
		++idx;
	}
	++pos;
	return ( index );
}

IDirective* parseReturnDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("invalid number of arguments for return directive");

	Return* return_dir = new Return();
	std::stringstream ss(tokens[pos++]->data);
	int tmp;
	ss >> tmp;
	if (ss.fail() || ss.eof() == false || tmp < 0 || tmp > 999)
	{
		delete return_dir;
		throw DirectiveException("invalid argument for return directive");
	}
	return_dir->setCode(tmp);
	if (pos >= tokensSize)
	{
		delete return_dir;
		throw DirectiveException("incomplete return directive - missing \";\"");
	}
	if (tokens[pos]->type == DIR_END)
	{
		++pos;
		return ( return_dir );
	}
	if (isValidURL(tokens[pos]->data) == false)
	{
		delete return_dir;
		throw DirectiveException("invalid argument for return directive - invalid url");
	}
	return_dir->setUrl(strdup(tokens[pos++]->data));
	if (tokens[pos]->type != DIR_END)
	{
		// <<"tokens[pos] " << tokens[pos] << std::endl;
		// <<"pos " << pos << std::endl;
		// <<"tokens[pos]->type " << tokens[pos]->type << std::endl;
		// << return_dir->getUrl() << std::endl;
		// <<"DIR_END " << DIR_END << std::endl;
		delete return_dir;
		throw DirectiveException("incomplete return directive - missing \";\"");
	}
	else
		++pos;
	return ( return_dir );
}

IDirective* parseClientMaxBodySizeDirective(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("incomplete clinet_max_body_size directive - missing size number");

	ClientMaxBodySize* clientMaxBodySize = new ClientMaxBodySize();
	std::stringstream ss(tokens[pos++]->data);
	unsigned long long tmp;
	ss >> tmp;
	if (ss.fail() || ss.eof() == false)
	{
		delete clientMaxBodySize;
		throw DirectiveException("invalid argument for client_max_body_size directive - invalid number");
	}
	clientMaxBodySize->setSize(tmp);
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete clientMaxBodySize;
		throw DirectiveException("incomplete client_max_body_size directive - missing \";\"");
	}
	else
		++pos;
	return ( clientMaxBodySize );
}

IDirective* parseAutoIndex(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("incomplete autoindex directive");

	AutoIndex* autoindex = new AutoIndex();
	if (strcmp(tokens[pos]->data, "on") == 0)
		autoindex->setState(true);
	else if (strcmp(tokens[pos]->data, "off") == 0)
		autoindex->setState(false);
	else
	{
		delete autoindex;
		throw DirectiveException("invalid content for autoindex directive");
	}
	++pos;
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete autoindex;
		throw DirectiveException("incomplete autoindex directive - missing \";\"");
	}
	else
		++pos;
	return ( autoindex );
}

IDirective* parseUploadDirective(std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("incomplete autoindex directive");

	Upload* upload = new Upload();
	if (strcmp(tokens[pos]->data, "on") == 0)
		upload->setState(true);
	else if (strcmp(tokens[pos]->data, "off") == 0)
		upload->setState(false);
	else {
		delete upload;
		throw DirectiveException("invalid content for upload directive");
	}
	++pos;
	if (pos >= tokensSize || tokens[pos]->type != DIR_END) {
		delete upload;
		throw DirectiveException("incomplete upload directive - missing \";\"");
	}
	else
		++pos;
	return ( upload );
}

IDirective* parseUploadLocationDirective(std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	if (pos >= tokensSize || tokens[pos]->type == DIR_END)
		throw DirectiveException("incomplete upload_location directive");

	UploadLocation* uploadLocation = new UploadLocation();
	uploadLocation->setLocation(strdup(tokens[pos++]->data));
	if (pos >= tokensSize || tokens[pos]->type != DIR_END)
	{
		delete uploadLocation;
		throw DirectiveException("incomplete root directive - missing \";\"");
	}
	++pos;
	// << ">>>>>>> " << uploadLocation->getLocation() << std::endl;
	return ( uploadLocation );
}

IDirective* createNode(
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize)
{
	char types[][21] = {"server", "listen", "server_name",
		"error_page", "client_max_body_size", "location", "root",
		"limit_except", "return", "index", "autoindex", "upload",
		"upload_location"};
	IDirective* (*parsers[])(std::vector<t_token*>&,unsigned int&,unsigned int) = {
		parseServerBlock,
		parseListenDirective,
		parseServerName,
		parseErrorPage,
		parseClientMaxBodySizeDirective,
		parseLocationBlock,
		parseRootDirective,
		parseLimitExceptDirective,
		parseReturnDirective,
		parseIndexDirective,
		parseAutoIndex,
		parseUploadDirective,
		parseUploadLocationDirective
	};

	for (int i = 0; i < 13; i++)
	{
		if (strcmp(types[i], tokens[pos]->data) == 0)
		{
			++pos;
			return ( parsers[i](tokens, pos, tokensSize) );
		}
	}
	throw DirectiveException("unknown directive");
}

void consumeDirectives(
		BlockDirective* block,
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize,
		TOKEN end)
{
	if (end == HTTP_END) {
		while (pos < tokensSize)
			block->addDirective(createNode(tokens, pos, tokensSize));
	}
	else {
		while (pos < tokensSize && tokens[pos]->type != end)
			block->addDirective(createNode(tokens, pos, tokensSize));
	}
}
