#ifndef CFG_PARSER
#define CFG_PARSER

#include <iostream>
#include <fstream>
#include <vector>

enum TOKEN {
	STRING,
	DIR_START,
	DIR_END,
	BLOCK_END,
};

typedef struct s_token {
	TOKEN type;
	char* data;
} t_token;

#endif
