#pragma once

#include "IDirective.hpp"

class ServerName : public IDirective {
	private:
		char** serverNames;

		ServerName(void);
		ServerName(const ServerName& other);
		ServerName& operator=(const ServerName& other);	

	public:
		ServerName(char** serverNames);
		~ServerName(void);
		DIRTYPE getType(void) const;
};

