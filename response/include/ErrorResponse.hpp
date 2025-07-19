#pragma once
#include "Response.hpp"
#include "../include/ResponseHandler.hpp"
class Connection;
class ErrorResponse {
public:
    static Response createErrorResponse(int statusCode, const std::string& message = "");
    static Response createErrorResponseWithMapping(Connection* conn, int statusCode, const std::string& message = "");
    static Response createMethodNotAllowedResponse(Connection* conn, const std::vector<std::string>& allowedMethods);
    static Response createNotFoundResponse(Connection* conn);
    static Response createForbiddenResponse();
    static Response createInternalErrorResponse();
}; 