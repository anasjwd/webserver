#!/bin/bash

for CLASS in $@; do
	CPP=${CLASS}.cpp
	rm -f $CPP
	cat > $CPP << EOF
#pragma once

#include "cfg_parser.hpp"

$CLASS:$CLASS(void)
{
	
}

$CLASS:~$CLASS(void)
{
	
}

DIRTYPE $CLASS:getType(void) const
{
	return ;
}

EOF
done
