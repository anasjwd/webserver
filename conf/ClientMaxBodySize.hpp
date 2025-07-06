#pragma once

#include "IDirective.hpp"

class ClientMaxBodySize : public IDirective {
	private:
		unsigned int size;

		ClientMaxBodySize(const ClientMaxBodySize& other);
		ClientMaxBodySize& operator=(const ClientMaxBodySize& other);

	public:
		ClientMaxBodySize(void);
		~ClientMaxBodySize(void);
		DIRTYPE getType(void) const;
		void setSize(unsigned int value);

		// TODO: Added by achraf!
		unsigned int getSize() {return size;}
};

