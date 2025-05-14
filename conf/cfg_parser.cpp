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

	(void)ac;
	char* content = readCfgFileContent(av[1]);
	tokens = tokenize(content);
	tokensNum = tokens.size();
	for (unsigned int i = 0; i < tokensNum; i++)
	{
		if (tokens[i]->type == STRING)
			std::cout << "STRING: " << tokens[i]->data << std::endl;
		else if (tokens[i]->type == DIR_START)
			std::cout << "DIR_START: {" << std::endl;
		else if (tokens[i]->type == DIR_END)
			std::cout << "DIR_END: }" << std::endl;
		else if (tokens[i]->type == BLOCK_END)
			std::cout << "BLOCK_END: ;" << std::endl;
		else if (tokens[i]->type == COLON)
			std::cout << "COLON: :" << std::endl;
		else if (tokens[i]->type == EQUAL)
			std::cout << "EQUAL: =" << std::endl;
		delete[] tokens[i]->data;
		delete tokens[i];
	}
	delete[] content;
	return 0;
}
