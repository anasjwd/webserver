#include "cfg_parser.hpp"
#include <cstring>

bool inStr(const char c, const char* charset)
{
	for (int i = 0; charset[i] != '\0'; i++)
	{
		if (charset[i] == c)
			return true;
	}
	return false;
}

bool isSep(const char c, const char* charset)
{
	if (inStr(c, charset))
		return true;
	return false;
}

char* grabWord(char* str, int& startToUpdate)
{
	int idx = startToUpdate;
	int wordLength;
	char* wordHolder;
	bool insideQuotes = false;
	char quotesType;

	while ((isSep(str[idx], " \t\n{};#") == false) && str[idx]) // stop at the second quote | handle the quotes inside quotes
	{
		if (str[idx] == '\'' || str[idx] == '\"')
			skipQuotedString(str, idx);
		idx++;
	}
	wordLength = idx - startToUpdate;
	wordHolder = new char[wordLength + 1];
	strncpy(wordHolder, str + startToUpdate, wordLength);
	wordHolder[wordLength] = '\0';
	startToUpdate = idx;
	return wordHolder;
}

char* grabComment(char* str, int& startToUpdate)
{
	int idx = startToUpdate;
	int commentLength;
	char* commentHolder;
	
	while (isSep(str[idx], "\n") == false && str[idx])
		idx++;
	commentLength = idx - startToUpdate;
	commentHolder = new char[commentLength + 1];
	strncpy(commentHolder, str + startToUpdate, commentLength);
	commentHolder[commentLength] = '\0';
	startToUpdate = idx;
	return commentHolder;
}

std::vector<char*> splitString(char* content)
{
	int idx = 0;
	std::vector<char*> words;
	char* wordHolder;

	while (content[idx] != '\0')
	{
		while (isSep(content[idx], " \t\n"))
			idx++;
		if (inStr(content[idx], "{};"))
		{
			wordHolder = new char[2];
			wordHolder[0] = content[idx++];
			wordHolder[1] = '\0';
		}
		else if (content[idx] == '#')
			wordHolder = grabComment(content, idx);
		else
			wordHolder = grabWord(content, idx);
		words.push_back(wordHolder);
	}
	return words;
}

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

std::vector<t_token*> tokenize(std::vector<char*>& words)
{
	std::vector<t_token*> tokens;
	t_token* token;
	unsigned int wordsNum = words.size();

	for (unsigned int i = 0; i < wordsNum; i++)
	{
		if (words[i][0] == '#')
		{
			delete[] words[i];
			continue ;
		}
		token = new t_token;
		token->data = NULL;
		if (words[i][0] == ';')
		{
			token->type = BLOCK_END;
			delete[] words[i];
		}
		else if (words[i][0] == '{')
		{
			token->type = DIR_START;
			delete[] words[i];
		}
		else if (words[i][0] == '}')
		{
			token->type = DIR_END;
			delete[] words[i];
		}
		else {
			token->type = STRING;
			token->data = words[i];
		}
		tokens.push_back(token);
	}
	return tokens;
}

int main(void)
{
	std::vector<char*> words;
	std::vector<t_token*> tokens;
	unsigned int tokensNum;

	char* content = readCfgFileContent("test.conf");
	words = splitString(content);
	tokens = tokenize(words);
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
		delete[] tokens[i]->data;
		delete tokens[i];
	}
	delete[] content;
	return 0;
}
