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

int main(int ac, char** av)
{
	std::vector<t_token*> tokens;
	unsigned int tokensNum;

	if (ac < 2)
	{
		std::cout << "config file\n";
		return 1;
	}
	char* content = readCfgFileContent(av[1]);
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
	return 0;
}
