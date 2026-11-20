#include "Lexer.h"

const std::unordered_set<std::string> Lexer::KEYWORDS = {
    "array", "boolean", "char", "else", "false",
    "for", "function", "if", "integer", "print",
    "return", "string", "true", "void", "while"
};

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source),
      filename_(filename),
      pos_(0),
      line_(1),
      col_(1)
{
}

void Lexer::reportError(const std::string& message, int line, int col) {
    Diagnostic diagnostic;

    diagnostic.location.filename = filename_;
    diagnostic.location.line = line;
    diagnostic.location.col = col;
    diagnostic.message = message;

    diagnostics_.push_back(diagnostic);
}

 // Navigation helpers

 bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
 }

 char Lexer::peek(std::size_t offset) const {
    if (pos_ + offset >= source_.size()) {
        return '\0';
    }

    return source_[pos_ + offset];
 }

 char Lexer::advance() {

    if (isAtEnd()) {
        return '\0';
    }

    char current = source_[pos_++];

    if (current == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }

    return current;
 }


 // Token creation

 Token Lexer::makeToken(
        TokenType type,
        std::size_t startPos,
        int startLine,
        int startCol
    ) const {

        Token token;
        token.type = type;
        token.lexeme = source_.substr(startPos, pos_ - startPos);
        token.location.filename = filename_;
        token.location.line = startLine;
        token.location.col = startCol;

        return token;
    }


    // Character helpers

    bool Lexer::isAlpha(char c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool Lexer::isDigit(char c){
        return c >= '0' && c<= '9';
    }

    bool Lexer::isAlphaNumeric(char c){
        return isAlpha(c) || isDigit(c);
    }


    // Scanning functions

    Token Lexer::scanWhiteSpace(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        while (peek() == ' ' || peek() == '\n' || peek() == '\t' || peek() == '\r'){
            advance();
        }

        return makeToken(
            TokenType::WHITESPACE,
            startPos,
            startLine,
            startCol
        );
    }


    Token Lexer::scanIdentifierOrKeyword(){
        
        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        // read the entire identifier
        while(isAlphaNumeric(peek())){
            advance();
        }

        // extract the identifier text
        std::string lexeme = source_.substr(startPos, pos_ - startPos);

        // check maximum identifier length
        if (lexeme.length() > MAX_IDENT_LENGTH){
            reportError("identifier exceeds 256 characters", startLine, startCol);

            return makeToken(
                TokenType::INVALID,
                startPos, 
                startLine, 
                startCol
            );
        }

        // check whether it's a keyword
        if (KEYWORDS.find(lexeme) != KEYWORDS.end()){
            return makeToken(
                TokenType::KEYWORD,
                startPos, 
                startLine, 
                startCol
            );
        }

        return makeToken(
                TokenType::IDENT,
                startPos, 
                startLine, 
                startCol
            );
    }


    Token Lexer::scanNumber(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        while (isDigit(peek())){
            advance();
        }

        return makeToken(
            TokenType::INT_LIT,
            startPos,
            startLine,
            startCol
        );
    }

    Token Lexer::scanString(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        advance(); // consume opening "

        std::size_t length = 0;
        bool tooLong = false;

        while (!isAtEnd() && peek() != '"' && peek() != '\n' && peek() != '\r'){

            if (peek() == '\\') {
                advance(); // consume backslash

                if (isAtEnd() || peek() == '\n' || peek() == '\r'){
                    break;
                }

                advance(); // consume escape characters
            }
            else {
                advance();
            }

            length++;

            if (length > MAX_STRING_LENGTH) {
                tooLong = true;
            }
        }

        if (peek() == '"'){
            advance(); //consume closing "

            if (tooLong){
                reportError("string literal exceeds 256 characters", startLine, startCol);

                return makeToken(
                    TokenType::INVALID,
                    startPos,
                    startLine,
                    startCol
                );
            }

            return makeToken(
                TokenType::STRING_LIT,
                startPos,
                startLine,
                startCol
            );
        }

        reportError("unterminated string literal", startLine, startCol);

        return makeToken(
            TokenType::UNTERMINATED_STRING,
                startPos,
                startLine,
                startCol
        );
    }


    Token Lexer::scanChar(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        advance(); // consume opening '

        // check for empty chracter or newline or EOF
        if (isAtEnd() || peek() == '\'' || peek() == '\n' || peek() == '\r'){
            if (peek() == '\'') {
            advance();   // consume the second ' of ''
        }
        
            reportError("invalid character literal", startLine, startCol);
            
            return makeToken(
                TokenType::INVALID,
                startPos,
                startLine,
                startCol
            );
        }

        // consume one logical chracter
        if (peek() == '\\') {
            advance(); // consume backslash

            if (isAtEnd() || peek() == '\n' || peek() == '\r') {
                reportError("invalid character literal", startLine, startCol);
                
                return makeToken(
                    TokenType::INVALID,
                    startPos, 
                    startLine, 
                    startCol
                );
            }

            advance(); // consume escaped character

        }
        else {
            advance(); // consume regular chracter;
        }

        // check for closing '
        if (peek() == '\''){
            advance(); // consume closing '
            return makeToken(
                TokenType::CHAR_LIT,
                startPos, 
                startLine, 
                startCol
            );
        }

        // invalid character literal: consume the remaining part
        while (!isAtEnd() && peek() != '\'' && peek() != '\n' && peek() != '\r'){
            advance();
        }

        if (peek() == '\''){
            advance(); // consume closing '
        }

        reportError("invalid character literal", startLine, startCol);

        return makeToken(
            TokenType::INVALID,
            startPos, 
            startLine, 
            startCol
        );
    }

    Token Lexer::scanComment(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        // single-line comment 
        if (peek(1) == '/') {

            advance(); // consume first /
            advance(); // consume second /

            while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
                advance();
            }

            return makeToken(
                TokenType::COMMENT,
                startPos,
                startLine,
                startCol
            );
        }

        advance(); // consume /
        advance(); // consume *

        // block comment
        while (!isAtEnd()){
            if (peek() == '*' && peek(1) == '/'){
                advance(); // consume *
                advance(); // consume /

                return makeToken(
                    TokenType::COMMENT,
                    startPos,
                    startLine,
                    startCol
                );
            }

            advance();
        }

        // block comment was never closed
        reportError("unterminated block comment", startLine, startCol);

        return makeToken(
            TokenType::UNTERMINATED_COMMENT,
            startPos,
            startLine,
            startCol
        );

    }


    Token Lexer::scanOperatorOrDelimiter(){

        std::size_t startPos = pos_;
        int startLine = line_;
        int startCol = col_;

        // check two-chracter operators
        std::string twoChars = source_.substr(pos_, 2);

        if (twoChars == "<=" || twoChars == ">=" ||
        twoChars == "==" || twoChars == "!=" ||
        twoChars == "&&" || twoChars == "||" ||
        twoChars == "++" || twoChars == "--") {

            advance();
            advance();

            return makeToken(
                TokenType::OPERATOR,
                startPos, 
                startLine, 
                startCol
            );
        }

        // check one-chracter operators
        if (peek() == '+' || peek() == '-' || 
        peek() == '*' || peek() == '/' ||
        peek() == '%' || peek() == '^' ||
        peek() == '<' || peek() == '>' ||
        peek() == '=' || peek() == '!') {

            advance();

            return makeToken(
                TokenType::OPERATOR,
                startPos, 
                startLine, 
                startCol
            );
        }

        // check delimiters
        if (peek() == '(' || peek() == ')' ||
        peek() == '[' || peek() == ']' ||
        peek() == '{' || peek() == '}' ||
        peek() == ',' || peek() == ';' ||
        peek() == ':') {

            advance();

            return makeToken(
                TokenType::DELIMITER,
                startPos, 
                startLine, 
                startCol
            );
        }

        // unknown chracter
        char bad = peek();

        advance();

        reportError(
            "invalid character '" + std::string(1, bad) + "'",
            startLine,
            startCol
        );

        return makeToken(
            TokenType::INVALID,
            startPos, 
            startLine, 
            startCol
        );
    }

    Token Lexer::nextToken(){

        // end of file
        if (isAtEnd()) {
            return makeToken(
            TokenType::END_OF_FILE,
            pos_, 
            line_, 
            col_
        );
    }

        // Whitespace
        if (peek() == ' ' || peek() == '\t' ||
        peek() == '\n' || peek() == '\r') {
            return scanWhiteSpace();
        }

        
        // Comments
        if (peek() == '/' && 
        (peek(1) == '/' || peek(1) == '*')) {
            return scanComment();
        }


        // String literals
        if (peek() == '"') {
            return  scanString();
        }

        // Char literals
        if (peek() == '\'') {
            return  scanChar();
        }

        // Identifier or keyword
        if (isAlpha(peek())) {
            return  scanIdentifierOrKeyword();
        }

        // Integer literals
        if (isDigit(peek())) {
            return  scanNumber();
        }

        // Operators, delimiters, or invalid characters
        return  scanOperatorOrDelimiter();
    }


    std::vector<Token> Lexer::tokenize() {

        std::vector <Token> tokens;

        while (true) {
            Token token = nextToken();

            tokens.push_back(token);

            if (token.type == TokenType::END_OF_FILE) {
                break;
            }
        }

        return tokens;
    }

    // Constants

    const std::size_t Lexer::MAX_IDENT_LENGTH = 256;
    const std::size_t Lexer::MAX_STRING_LENGTH = 256;

