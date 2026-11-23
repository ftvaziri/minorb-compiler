#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <initializer_list>

#include "Ast.h"
#include "Diagnostic.h"
#include "Token.h"

class Parser {
public:
    // WHITESPACE, COMMENT, INVALID, UNTERMINATED_STRING and
    // UNTERMINATED_COMMENT tokens are filtered out here (the lexer has
    // already reported errors for the invalid ones). If the input has no
    // END_OF_FILE token, one is appended, so tokens_ is never empty and
    // always ends with END_OF_FILE.
    // The caller keeps the full token vector for the highlighter.
    explicit Parser(std::vector<Token> tokens);

    // Returns the Program root. Never returns nullptr.
    std::unique_ptr<Node> parse();

    const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

private:
    // State
    std::vector<Token> tokens_;      // filtered
    std::size_t pos_ = 0;
    std::vector<Diagnostic> diagnostics_;

    // Token helpers
    const Token& peek(std::size_t offset = 0) const; // past the end -> END_OF_FILE
    const Token& advance();                          // does not move past END_OF_FILE
    bool isAtEnd() const;                            // current token is END_OF_FILE
    bool check(TokenType type) const;
    bool checkLexeme(const std::string& lexeme) const;
    bool matchLexeme(const std::string& lexeme);     // consumes on success
    bool expectLexeme(const std::string& lexeme,
                      const std::string& message);   // reports error on failure

    // Errors
    void reportError(const std::string& message, const SourceLocation& location);
    std::unique_ptr<Node> makeError(const SourceLocation& location);
    // Skips tokens until just after ';' (consumed), or before '}' or one of
    // if / for / return / print (not consumed), or END_OF_FILE.
    void synchronize();

    // Structure
    std::unique_ptr<Node> parseProgram();
    // Reads IDENT ':' and builds the Identifier node, then dispatches to
    // parseVarDeclTail / parseFuncDeclTail. Also used by parseBlockItem
    // with allowFunction = false (local_declaration has no function form).
    std::unique_ptr<Node> parseDeclaration(bool allowFunction = true);
    std::unique_ptr<Node> parseVarDeclTail(std::unique_ptr<Node> name);
    std::unique_ptr<Node> parseFuncDeclTail(std::unique_ptr<Node> name);
    std::unique_ptr<Node> parseParam();

    // Types
    std::unique_ptr<Node> parseAtomicType();
    std::unique_ptr<Node> parseType();
    std::unique_ptr<Node> parseParameterType();
    std::unique_ptr<Node> parseReturnType();

    // Initializers
    std::unique_ptr<Node> parseInitializer();
    std::unique_ptr<Node> parseArrayInit();

    // Statements
    std::unique_ptr<Node> parseBlock();
    std::unique_ptr<Node> parseBlockItem();  // LL(2): IDENT followed by ':' -> declaration
    std::unique_ptr<Node> parseStatement();
    std::unique_ptr<Node> parseReturn();
    std::unique_ptr<Node> parsePrint();
    std::unique_ptr<Node> parseIf();
    std::unique_ptr<Node> parseFor();

    // Expressions (lowest to highest precedence)
    using ParseFn = std::unique_ptr<Node> (Parser::*)();
    std::unique_ptr<Node> parseBinaryLevel(ParseFn next,
                                           std::initializer_list<const char*> ops);
    std::unique_ptr<Node> parseExpression();
    std::unique_ptr<Node> parseAssignment();
    std::unique_ptr<Node> parseOr();
    std::unique_ptr<Node> parseAnd();
    std::unique_ptr<Node> parseEquality();
    std::unique_ptr<Node> parseRelational();
    std::unique_ptr<Node> parseAdditive();
    std::unique_ptr<Node> parseMultiplicative();
    std::unique_ptr<Node> parsePower();
    std::unique_ptr<Node> parseUnary();
    std::unique_ptr<Node> parsePostfix();
    std::unique_ptr<Node> parsePrimary();
};

#endif // PARSER_H