#pragma once

#include "IDirective.hpp"

class Server : public IDirective {
	private:
		bool isDefaultServer;
		std::vector<IDirective*> directives;
		Server(void);
		Server(const Server& other);
		Server& operator=(const Server& other);
	public:
		Server(bool isDefaultServer);
		~Server(void);
		DIRTYPE getType(void) const;
		void addDirective(IDirective* dir);
};
