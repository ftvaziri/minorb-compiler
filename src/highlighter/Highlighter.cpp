
#include "Highlighter.h"


// Identifier Classification


void Highlighter::registerIdentifier(
    const SourceLocation& location,
    HighlightClass category
) {
    auto key = std::make_pair(
        location.line,
        location.col
    );

    // Do not overwrite an existing classification.
    // This is important for function identifiers.
    identifierCategories_.emplace(key, category);
}



// AST Traversal


void Highlighter::analyzeAST(const Node* node) {

    if (node == nullptr) {
        return;
    }

    // Function declaration:
    // children[1] is the function name.
    if (node->kind == NodeKind::FuncDecl) {

        if (node->children.size() > 1 &&
            node->children[1] != nullptr) {

            registerIdentifier(
                node->children[1]->location,
                HighlightClass::Function
            );
        }
    }

    // Function call:
    // children[0] is the callee.
    if (node->kind == NodeKind::CallExpr) {

        if (!node->children.empty() &&
            node->children[0] != nullptr &&
            node->children[0]->kind == NodeKind::Identifier) {

            registerIdentifier(
                node->children[0]->location,
                HighlightClass::Function
            );
        }
    }

    // Ordinary identifiers are variables.
    // emplace prevents overwriting Function classifications.
    if (node->kind == NodeKind::Identifier) {

        registerIdentifier(
            node->location,
            HighlightClass::Variable
        );
    }

    // Visit all children, including children of ErrorNode.
    for (const auto& child : node->children) {
        analyzeAST(child.get());
    }
}



// Type Keyword Detection


bool Highlighter::isTypeKeyword(
    const std::string& lexeme
) const {

    return
        lexeme == "integer" ||
        lexeme == "boolean" ||
        lexeme == "char" ||
        lexeme == "string" ||
        lexeme == "void" ||
        lexeme == "array";
}



// Token Classification


HighlightClass Highlighter::classifyToken(
    const Token& token
) const {

    switch (token.type) {

        case TokenType::WHITESPACE:
            return HighlightClass::Plain;

        case TokenType::COMMENT:
            return HighlightClass::Comment;

        case TokenType::KEYWORD:

            if (isTypeKeyword(token.lexeme)) {
                return HighlightClass::Type;
            }

            if (token.lexeme == "true" ||
                token.lexeme == "false") {
                return HighlightClass::Literal;
            }

            return HighlightClass::Keyword;

        case TokenType::IDENT: {

            auto key = std::make_pair(
                token.location.line,
                token.location.col
            );

            auto it = identifierCategories_.find(key);

            if (it != identifierCategories_.end()) {
                return it->second;
            }

            // Default classification when AST
            // did not contain this identifier.
            return HighlightClass::Variable;
        }

        case TokenType::INT_LIT:
        case TokenType::CHAR_LIT:
        case TokenType::STRING_LIT:
            return HighlightClass::Literal;

        case TokenType::OPERATOR:
            return HighlightClass::Operator;

        case TokenType::DELIMITER:
            return HighlightClass::Delimiter;

        case TokenType::INVALID:
        case TokenType::UNTERMINATED_STRING:
        case TokenType::UNTERMINATED_COMMENT:
            return HighlightClass::Error;

        case TokenType::END_OF_FILE:
            return HighlightClass::Plain;
    }

    return HighlightClass::Plain;
}



// Main Highlighting Function


std::vector<HighlightedToken> Highlighter::highlight(
    const std::vector<Token>& tokens,
    const Node* ast
) {

    // Clear previous analysis results.
    identifierCategories_.clear();

    // First: analyze AST.
    analyzeAST(ast);

    // Second: classify the complete token stream.
    std::vector<HighlightedToken> result;

    result.reserve(tokens.size());

    for (const Token& token : tokens) {

        // EOF has no source text to render.
        if (token.type == TokenType::END_OF_FILE) {
            continue;
        }

        HighlightedToken highlighted;

        highlighted.token = token;
        highlighted.category = classifyToken(token);

        result.push_back(highlighted);
    }

    return result;
}
