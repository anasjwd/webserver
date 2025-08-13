#ifndef CFG_PARSER
#define CFG_PARSER

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sstream>
#include "Exceptions.hpp"
#define NONE '\0'

enum TOKEN {
	STRING,
	BLOCK_START,
	BLOCK_END,
	DIR_END,
	COLON,
	EQUAL,
	HTTP_END
};

/*enum DIRTYPE {
	HTTP,
	SERVER,
	LISTEN,
	SERVER_NAME,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	LOCATION,
	ROOT,
	LIMIT_EXCEPT,
	RETURN,
	INDEX,
	AUTOINDEX,
	DENY,
	ALLOW
};*/

typedef struct s_token {
	TOKEN type;
	char* data;
} t_token;

#include "IDirective.hpp"
#include "BlockDirective.hpp"

#include "Http.hpp"
#include "Server.hpp"
#include "Listen.hpp"
#include "ServerName.hpp"
#include "ErrorPage.hpp"
#include "ClientMaxBodySize.hpp"
#include "Location.hpp"
#include "Root.hpp"
#include "LimitExcept.hpp"
#include "Return.hpp"
#include "Index.hpp"
#include "AutoIndex.hpp"
#include "Upload.hpp"
#include "UploadLocation.hpp"
#include <algorithm>

#include <algorithm>

Http* parseConfig(const char* fileName);

std::vector<t_token*> tokenize(char* content);

Http* parser(std::vector<t_token*>& tokens);

void consumeDirectives(
		BlockDirective* block,
		std::vector<t_token*>& tokens,
		unsigned int& pos,
		unsigned int tokensSize,
		TOKEN end);

bool isValidURI(char* uri);
bool isHostAndPort(std::vector<t_token*>& tokens, unsigned int pos, unsigned int tokensSize);
bool isPortOnly(std::vector<t_token*>& tokens, unsigned int pos);
bool isIPv4(const char* addr);
bool isValidMethod(char* method);
bool isValidURL(char* url);

#endif
