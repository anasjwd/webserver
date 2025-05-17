#pragma once

#include "cfg_parser.hpp"

class DirectiveFactory {
	private:
		DirectiveFactory(void);
		DirectiveFactory(const DirectiveFactory& other);
		DirectiveFactory& operator=(const DirectiveFactory& other);

	public:
		static IDirective* create(std::vector<t_token*>& tokens);

		static void parseBlock(std::vector<IDirective*> directives,
				std::vector<t_token*>& tokens, int& pos);

		static IDirective* parseServerBlock(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseLocationBlock(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseLimitExceptBlock(std::vector<t_token*>& tokens, int& pos);

		static IDirective* parseListenDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseListenDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseServerNameDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseErrorPageDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseClientMaxBodySizeDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseRootDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseReturnDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseIndexDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseAutoIndexDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseAllowDirective(std::vector<t_token*>& tokens, int& pos);
		static IDirective* parseDenyDirective(std::vector<t_token*>& tokens, int& pos);
};
