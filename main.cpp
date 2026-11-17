#include <iostream>
#include <string>
#include <vector>
#include "src/lexer/Lexer.h"

const char* toString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD:              return "KEYWORD";
        case TokenType::IDENT:                return "IDENT";
        case TokenType::INT_LIT:              return "INT_LIT";
        case TokenType::CHAR_LIT:             return "CHAR_LIT";
        case TokenType::STRING_LIT:           return "STRING_LIT";
        case TokenType::OPERATOR:             return "OPERATOR";
        case TokenType::DELIMITER:            return "DELIMITER";
        case TokenType::COMMENT:              return "COMMENT";
        case TokenType::WHITESPACE:           return "WHITESPACE";
        case TokenType::INVALID:              return "INVALID";
        case TokenType::UNTERMINATED_STRING:  return "UNTERMINATED_STRING";
        case TokenType::UNTERMINATED_COMMENT: return "UNTERMINATED_COMMENT";
        case TokenType::END_OF_FILE:          return "END_OF_FILE";
    }
    return "UNKNOWN";
}

// show newlines/tabs visibly in the lexeme
std::string escapeForDisplay(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\n')      out += "\\n";
        else if (c == '\t') out += "\\t";
        else if (c == '\r') out += "\\r";
        else                out += c;
    }
    return out;
}

void runTest(const std::string& name, const std::string& source) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "input: " << escapeForDisplay(source) << "\n";

    Lexer lexer(source, "test.bminor");
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token& token : tokens) {
        if (token.type == TokenType::WHITESPACE) continue;

        std::cout << "  " << toString(token.type)
                  << " | \"" << escapeForDisplay(token.lexeme) << "\""
                  << " | " << token.location.line
                  << ":" << token.location.col << "\n";
    }
    std::cout << "\n";
}

int main() {
    // strings
    runTest("empty string",         R"("")");
    runTest("string with escape",   R"("hi\n")");
    runTest("unterminated string",  R"("abc)");

    // comments
    runTest("line comment",         "// c");
    runTest("block comment",        "/* x */");
    runTest("unterminated block",   "/* x");
    runTest("multiline block",      "/* a\nb */ x");

    // operators
    runTest("less-equal",           "a<=b");
    runTest("invalid char",         "x = 5 @ 3");

    // chars
    runTest("char a",               "'a'");
    runTest("char newline",         R"('\n')");
    runTest("empty char",           "''");

    // keyword vs identifier
    runTest("return keyword",       "return");
    runTest("returnx identifier",   "returnx");
    runTest("underscore ident",     "_x1 Fog123");

    // a small realistic program
    runTest("declaration",          "integer age = 25;");
    runTest("function",
        "square: function integer ( x: integer ) = {\n"
        "    return x^2; // square\n"
        "}\n");

    return 0;
}