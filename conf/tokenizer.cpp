#include "cfg_parser.hpp"

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

	while ((isSep(str[idx], " \t\n{};:#") == false || insideQuotes == true)
			&& str[idx] != '\0') // TODO: handle quotes
	{
		if (inStr(str[idx], "\'\"") && insideQuotes == false)
		{
			insideQuotes = true;
			quotesType = str[idx];
		}
		else if (quotesType == str[idx])
		{
			insideQuotes = false;
			quotesType = NONE;
		}
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
	
	while (isSep(str[idx], "\n") == false && str[idx]) // TODO: handle quotes
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
		if (inStr(content[idx], "{};:="))
		{
			wordHolder = new char[2];
			wordHolder[0] = content[idx++];
			wordHolder[1] = '\0';
		}
		else if (content[idx] == '#')
			wordHolder = grabComment(content, idx);
		else
			wordHolder = grabWord(content, idx);
		if (wordHolder[0] == '\0')
		{
			delete[] wordHolder;
			break;
		}
		words.push_back(wordHolder);
	}
	return words;
}

std::vector<t_token*> tokenize(char* content)
{
	std::vector<char*> words;
	std::vector<t_token*> tokens;
	t_token* token;
	unsigned int wordsNum;

	words = splitString(content);
	wordsNum = words.size();
	for (unsigned int i = 0; i < wordsNum; i++)
	{
		if (words[i][0] == '#')
		{
			delete[] words[i];
			continue ;
		}
		token = new t_token;
		token->data = NULL;
		switch (words[i][0]) {
			case ';':
				token->type = DIR_END;
				token->data = words[i];
				//delete[] words[i];
				break;
			case '{':
				token->type = BLOCK_START;
				token->data = words[i];
				//delete[] words[i];
				break;
			case '}':
				token->type = BLOCK_END;
				token->data = words[i];
				//delete[] words[i];
				break;
			case ':':
				token->type = COLON;
				token->data = words[i];
				//delete[] words[i];
				break;
			case '=':
				token->type = EQUAL;
				token->data = words[i];
				//delete[] words[i];
				break;
			default:
				token->type = STRING;
				token->data = words[i];
		}
		tokens.push_back(token);
	}
	return tokens;
}
