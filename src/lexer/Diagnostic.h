#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "Token.h"
#include <string>


struct Diagnostic {
    SourceLocation location;
    std::string message;
};


#endif