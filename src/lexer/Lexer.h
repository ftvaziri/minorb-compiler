#ifndef LEXER_H
#define LEXER_H

#include "Token.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <cstddef>
#include "Diagnostic.h"

class Lexer {


    public:

    // Constructor
    Lexer(const std::string& source, const std::string& filename);

    // Get the next token
    Token nextToken();

    // Get all tokens until END_OF_FILE
    std::vector<Token> tokenize();

    // Get all lexical diagnostics
    const std::vector<Diagnostic>& diagnostics() const {
    return diagnostics_;
}


    private:

    // State

    std::string source_;
    std::string filename_;

    std::size_t pos_;
    int line_;
    int col_;

    std::vector<Diagnostic> diagnostics_;


    // Navigation helpers

    char peek(std::size_t offset = 0) const;

    char advance();

    bool isAtEnd() const;

    void reportError(const std::string& message, int line, int col);


    // Token creation

    Token makeToken(
        TokenType type,
        std::size_t startPos,
        int startLine,
        int startCol
    ) const;


    // Scanning functions

    Token scanWhiteSpace();

    Token scanComment(); 

    Token scanIdentifierOrKeyword();

    Token scanNumber();

    Token scanString();

    Token scanChar();

    Token scanOperatorOrDelimiter();


    // Character helpers

    static bool isAlpha(char c);

    static bool isDigit(char c);

    static bool isAlphaNumeric(char c);

    // Constatnts 

    static const std::unordered_set<std::string> KEYWORDS;

    static const std::size_t MAX_IDENT_LENGTH;
    static const std::size_t MAX_STRING_LENGTH;


};


#endif // LEXER_H
