#pragma once

#include <cstdlib>

enum DIRTYPE {
	HTTP,
	SERVER,
	LISTEN,
	SERVER_NAME,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	LOCATION,
	ROOT,
	LIMIT_EXCEPT,
	RETURN,
	INDEX,
	AUTOINDEX,
	DENY,
	ALLOW
};

class IDirective {
	public:
		virtual ~IDirective(void) {};
		virtual DIRTYPE getType(void) const = 0;
};

