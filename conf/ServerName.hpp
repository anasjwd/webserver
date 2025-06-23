#pragma once

#include "IDirective.hpp"
#include <cstdlib>

class ServerName : public IDirective {
	private:
		char** serverNames;

		ServerName(const ServerName& other);
		ServerName& operator=(const ServerName& other);	

	public:
		ServerName(void);
		~ServerName(void);

		DIRTYPE getType(void) const;
		void setServerNames(char** value);
		void setServerName(char* value, unsigned int idx);
};

