#ifndef CFG_PARSER
#define CFG_PARSER

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#define NONE '\0'

enum TOKEN {
	STRING,
	DIR_START,
	DIR_END,
	BLOCK_END,
	COLON,
	EQUAL
};

enum DIRTYPE {
	LISTEN,
	SERVER_NAME
};

typedef struct s_token {
	TOKEN type;
	char* data;
} t_token;

class Directive {
	public:
		DIRTYPE getType(void) const = 0;
};

class Listen : public Directive {
	private:
		char* host;
		unsigned int port;

	public:
		Listen(void);
		~Listen(void);
		Listen(const Listen& other);
		Listen& operator=(const Listen& other);
		DIRTYPE getType(void) const;
};

class ServerName : public Directive {
	private:
		char** serverNames;
	public:
		ServerName(void);
		~ServerName(void);
		ServerName(const ServerName& other);
		ServerName& operator=(const ServerName& other);	
		DIRTYPE getType(void) const;
};

class ErrorPage : public Directive {
	private:
		unsigned int code;
		unsigned int responseCode;
		char* uri;
	public:
		ErrorPage(void);
		~ErrorPage(void);
		ErrorPage(const ErrorPage& other);
		ErrorPage operator=(const ErrorPage& other);
		DIRTYPE getType(void) const;
};

class ClientMaxBodySize : public Directive {
	private:
		unsigned int size;
	public:
		ClientMaxBodySize(void);
		~ClientMaxBodySize(void);
		ClientMaxBodySize(const ClientMaxBodySize& other);
		ClientMaxBodySize& operator=(const ClientMaxBodySize& other);
		DIRTYPE getType(void) const;
};

class Location : public Directive {
	private:
		char* uri;
		bool exactMatch;
		std::vector<Directive*> children;
	public:
		Location(void);
		~Location(void);
		Location(const Location& other);
		Location& operator=(const Location& other);
		DIRTYPE getType(void) const;
};

class Root : public Directive {
	private:
		char* path;
	public:
		Root(void);
		~Root(void);
		Root(const Root& other);
		Root& operator=(const Root& other);
		DIRTYPE getType(void) const;
};

class Return : public Directive {
	private:
		unsigned int code;
		char* url;
	public:
		Return(void);
		~Return(void);
		Return(const Return& other);
		Return& operator=(const Return& other);
		DIRTYPE getType(void) const;
};

std::vector<t_token*> tokenize(char* content);

#endif
