#include "cfg_parser.hpp"

char* readCfgFileContent(const char* filename)
{
	std::ifstream cfgFile;
	unsigned long contentLength;
	char* content;

	cfgFile.open(filename);
	if (!cfgFile.is_open())
	{
		std::cerr << "Error\n";
		return NULL; // TODO: throw an exception instead
	}
	cfgFile.seekg(0, std::ios::end);
	contentLength = cfgFile.tellg();
	cfgFile.seekg(0, std::ios::beg);
	content = new char[contentLength + 1];
	cfgFile.read(content, contentLength);
	content[contentLength] = '\0';
	cfgFile.close();
	return content;
}

Http* parseConfig(char* fileName)
{
	char* content = readCfgFileContent(av[1]);
	std::vector<t_token*> tokens;
	unsigned int tokensNum;

	tokens = tokenize(content);
	tokensNum = tokens.size();
	Http* http = parser(tokens);
	delete http;
	for (unsigned int i = 0; i < tokensNum; i++)
	{
		delete[] tokens[i]->data;
		delete tokens[i];
	}
	delete[] content;
	return ( http );
}
