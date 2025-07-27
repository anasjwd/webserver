#pragma once

#include "IDirective.hpp"
#include <string>

class CgiExtension : public IDirective {
private:
    std::string extension;
    std::string interpreter;

    CgiExtension(const CgiExtension& other);
    CgiExtension& operator=(const CgiExtension& other);

public:
    CgiExtension();
    ~CgiExtension();
    
    DIRTYPE getType() const;
    void setExtension(const std::string& ext);
    void setInterpreter(const std::string& interp);
    const std::string& getExtension() const;
    const std::string& getInterpreter() const;
    bool validate() const;
}; 