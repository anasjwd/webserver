#pragma once

#include <iostream>
#include <map>

class Response
{
    private:
        int _statusCode;
        std::string _statusMessage;
        std::map<std::string, std::string> _headers;
        std::string _body;
    public:
        Response();

        void setStatus(int code);
        void addHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& bodyContent);
        std::string build() const;
};
