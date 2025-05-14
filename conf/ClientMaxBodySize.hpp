#pragma once

#include "IDirective.hpp"

class ClientMaxBodySize : public IDirective {
	private:
		unsigned int size;

		ClientMaxBodySize(void);
		ClientMaxBodySize(const ClientMaxBodySize& other);
		ClientMaxBodySize& operator=(const ClientMaxBodySize& other);

	public:
		ClientMaxBodySize(unsigned int size);
		~ClientMaxBodySize(void);
		DIRTYPE getType(void) const;
};

