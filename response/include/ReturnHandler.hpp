#pragma once
#include "Response.hpp"
class Connection;
class ReturnHandler {
public:
    static Response handle(Connection* conn);
}; 