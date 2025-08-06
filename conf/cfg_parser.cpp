#include "cfg_parser.hpp"

char* readCfgFileContent(const char* filename)
{
	std::ifstream cfgFile;
	unsigned long contentLength;
	char* content;

	cfgFile.open(filename);
	if (!cfgFile.is_open())
	{
		std::cerr << "Error: Failed to open config file\n";
		return NULL;
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

Http* parseConfig(char* fileName) {
	char* content = readCfgFileContent(fileName);
	if (content == NULL)
		return ( NULL );
	std::vector<t_token*> tokens;
	unsigned int tokensNum;

	tokens = tokenize(content);
	tokensNum = tokens.size();
	Http* http = parser(tokens);
	for (unsigned int i = 0; i < tokensNum; i++)
	{
		delete[] tokens[i]->data;
		delete tokens[i];
	}
	delete[] content;
	return ( http );
}
