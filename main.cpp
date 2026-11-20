
#include <iostream>
#include <string>
#include <vector>
#include "src/lexer/Lexer.h"

void runTest(const std::string& source) {
    Lexer lexer(source, "test.bminor");
    lexer.tokenize();

    for (const Diagnostic& diagnostic : lexer.diagnostics()) {
        std::cout << diagnostic.location.filename
                  << ":" << diagnostic.location.line
                  << ":" << diagnostic.location.col
                  << ": error: " << diagnostic.message
                  << "\n";
    }
}

int main() {
    runTest("x = 5 @ 3");
    runTest("\"abc");
    runTest("/* x");
    runTest("'''");
    runTest("integer age = 25;");

    return 0;
}
