#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType { 
    WHITESPACE,
    COMMENT,
    KEYWORD,
    IDENT,
    INT_LIT,
    CHAR_LIT,
    STRING_LIT,
    OPERATOR,
    DELIMITER,
    INVALID,
    UNTERMINATED_STRING,
    UNTERMINATED_COMMENT,
    END_OF_FILE
};

struct SourceLocation {
    std::string filename;
    int line = 1;
    int col = 1;
};

struct Token {
    TokenType type;
    std::string lexeme;
    SourceLocation location;
};


#endif