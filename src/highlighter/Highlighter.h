
#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "Token.h"
#include "Ast.h"

#include <vector>
#include <map>
#include <utility>

// Categories used for syntax highlighting
enum class HighlightClass {
    Keyword,
    Type,
    Function,
    Variable,
    Literal,
    Operator,
    Delimiter,
    Comment,
    Error,
    Plain
};

// A token paired with its highlighting category
struct HighlightedToken {
    Token token;
    HighlightClass category;
};

class Highlighter {
public:

    // Analyze AST and classify the complete token stream
    std::vector<HighlightedToken> highlight(
        const std::vector<Token>& tokens,
        const Node* ast
    );

private:

    // Maps (line, column) to a highlighting category
    using LocationMap =
        std::map<std::pair<int, int>, HighlightClass>;

    LocationMap identifierCategories_;

    // Traverse AST recursively
    void analyzeAST(const Node* node);

    // Register an identifier's category
    void registerIdentifier(
        const SourceLocation& location,
        HighlightClass category
    );

    // Classify an individual token
    HighlightClass classifyToken(const Token& token) const;

    // Check whether a keyword represents a type
    bool isTypeKeyword(const std::string& lexeme) const;

};

#endif // HIGHLIGHTER_H
