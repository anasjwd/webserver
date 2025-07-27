#include "CgiExtension.hpp"

CgiExtension::CgiExtension() {
}

CgiExtension::~CgiExtension() {
}

DIRTYPE CgiExtension::getType() const {
    return CGI_EXTENSION;
}

void CgiExtension::setExtension(const std::string& ext) {
    extension = ext;
}

void CgiExtension::setInterpreter(const std::string& interp) {
    interpreter = interp;
}

const std::string& CgiExtension::getExtension() const {
    return extension;
}

const std::string& CgiExtension::getInterpreter() const {
    return interpreter;
}

bool CgiExtension::validate() const {
    return !extension.empty() && !interpreter.empty();
} 