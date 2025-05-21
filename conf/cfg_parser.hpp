#ifndef CFG_PARSER
#define CFG_PARSER

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#define NONE '\0'

enum TOKEN {
	STRING,
	BLOCK_START,
	BLOCK_END,
	DIR_END,
	COLON,
	EQUAL
};

enum DIRTYPE {
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
};

typedef struct s_token {
	TOKEN type;
	char* data;
} t_token;

#include "IDirective.hpp"
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
#include "Allow.hpp"
#include "Deny.hpp"

std::vector<t_token*> tokenize(char* content);

#endif
