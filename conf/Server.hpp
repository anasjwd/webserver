#pragma once

#include "BlockDirective.hpp"

class Server : public BlockDirective {
	private:
		Server(const Server& other);
		Server& operator=(const Server& other);

	public:
		Server(void);
		~Server(void);
		DIRTYPE getType(void) const;

		bool validate(void);
};
