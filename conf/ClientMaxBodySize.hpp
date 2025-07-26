#pragma once

#include "IDirective.hpp"

class ClientMaxBodySize : public IDirective {
	private:
		unsigned long long size;

		ClientMaxBodySize(const ClientMaxBodySize& other);
		ClientMaxBodySize& operator=(const ClientMaxBodySize& other);

	public:
		ClientMaxBodySize(void);
		~ClientMaxBodySize(void);
		DIRTYPE getType(void) const;
		void setSize(unsigned long long value);

		// TODO: Added by achraf!
		unsigned long long getSize() {return size;}
};

