#pragma once

#include "cfg_parser.hpp"

class Directive {
	public:
		virtual DIRTYPE getType(void) const = 0;
};

