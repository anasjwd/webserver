#include "cfg_parser.hpp"

bool isValidURI(char* uri)
{
	(void)uri;
	// TODO: implement this
	return ( true );
}

bool isValidURL(char* url)
{
	(void)url;
	// TODO: implement this
	return ( true );
}

bool isValidMethod(char* method)
{
	if (strcmp(method, "GET") == 0)
		return ( true );
	else if (strcmp(method, "POST") == 0)
		return ( true );
	else if (strcmp(method, "DELETE") == 0)
		return ( true );
	else
		return ( false );
}

bool isHostAndPort(std::vector<t_token*>& tokens, unsigned int pos, unsigned int tokensSize)
{
	if (strcmp(tokens[pos]->data, "localhost") != 0
			&& strcmp(tokens[pos]->data, "*") != 0
			&& isIPv4(tokens[pos]->data) == false)
		return ( false );
	++pos;
	if (pos >= tokensSize)
		return ( false );
	else if (tokens[pos]->type == COLON)
	{
		++pos;
		if (pos >= tokensSize || isPortOnly(tokens, pos) == false)
			return ( false );
	}
	return ( true );
}

bool isPortOnly(std::vector<t_token*>& tokens, unsigned int pos)
{
	std::stringstream ss(tokens[pos]->data);
	int port;

	ss >> port;
	if (ss.fail() == true || ss.eof() == false)
		return ( false );
	if (port < 0 || port > 65535)
		return ( false );
	return ( true );
}

bool isIPv4(const char* addr)
{
	int idx = 0;
	int octet;
	int numOfOctets = 0;

	while (addr[idx] != '\0')
	{
		octet = 0;
		while (addr[idx] != '.' && addr[idx] != '\0')
		{
			if (addr[idx] < '0' || addr[idx] > '9')
				return ( false );
			octet *= 10;
			octet += addr[idx] - '0';
			++idx;
		}
		if (octet > 255)
			return ( false );
		++numOfOctets;
		if (addr[idx] == '.')
		{
			idx++;
			if (addr[idx] == '\0')
				return ( false );
		}
	}
	if (numOfOctets != 4)
		return ( false );
	return ( true );
}

bool validMethod(char* method)
{
	if (strcmp(method, "GET") == 0)
		return ( true );
	if (strcmp(method, "POST") == 0)
		return ( true );
	if (strcmp(method, "DELETE") == 0)
		return ( true );
	return ( false );
}
