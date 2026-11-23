#include "Parser.h"

#include <utility>

namespace {
bool isAtomicTypeName(const std::string& s) {
    return s == "integer" || s == "boolean" || s == "char" || s == "string";
}
} // namespace


// 1. Constructor and token helpers


Parser::Parser(std::vector<Token> tokens) {
    for (Token& t : tokens) {
        switch (t.type) {
            case TokenType::WHITESPACE:
            case TokenType::COMMENT:
            case TokenType::INVALID:
            case TokenType::UNTERMINATED_STRING:
            case TokenType::UNTERMINATED_COMMENT:
                continue;   // lexer already reported errors for the invalid ones
            default:
                tokens_.push_back(std::move(t));
        }
    }

    if (tokens_.empty() || tokens_.back().type != TokenType::END_OF_FILE) {
        Token eof;
        eof.type = TokenType::END_OF_FILE;
        if (!tokens_.empty()) {
            eof.location = tokens_.back().location;
        }
        tokens_.push_back(std::move(eof));
    }
}

const Token& Parser::peek(std::size_t offset) const {
    std::size_t index = pos_ + offset;
    if (index >= tokens_.size()) {
        return tokens_.back();   // END_OF_FILE
    }
    return tokens_[index];
}

const Token& Parser::advance() {
    const Token& consumed = tokens_[pos_];
    if (!isAtEnd()) {
        pos_++;
    }
    return consumed;
}

bool Parser::isAtEnd() const {
    return tokens_[pos_].type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::checkLexeme(const std::string& lexeme) const {
    return peek().lexeme == lexeme;
}

bool Parser::matchLexeme(const std::string& lexeme) {
    if (checkLexeme(lexeme)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::expectLexeme(const std::string& lexeme, const std::string& message) {
    if (matchLexeme(lexeme)) {
        return true;
    }
    reportError(message, peek().location);
    return false;
}

void Parser::reportError(const std::string& message, const SourceLocation& location) {
    // Avoid cascades: only one error per source position.
    if (!diagnostics_.empty()) {
        const SourceLocation& last = diagnostics_.back().location;
        if (last.line == location.line && last.col == location.col) {
            return;
        }
    }

    Diagnostic diagnostic;
    diagnostic.location = location;
    diagnostic.message = message;
    diagnostics_.push_back(std::move(diagnostic));
}

std::unique_ptr<Node> Parser::makeError(const SourceLocation& location) {
    return makeNode(NodeKind::ErrorNode, location);
}


// 2. Expressions (from parsePrimary upward)


std::unique_ptr<Node> Parser::parsePrimary() {
    const SourceLocation location = peek().location;

    switch (peek().type) {
        case TokenType::IDENT: {
            const Token& t = advance();
            return makeNode(NodeKind::Identifier, t.location, t.lexeme);
        }
        case TokenType::INT_LIT: {
            const Token& t = advance();
            return makeNode(NodeKind::IntLiteral, t.location, t.lexeme);
        }
        case TokenType::CHAR_LIT: {
            const Token& t = advance();
            return makeNode(NodeKind::CharLiteral, t.location, t.lexeme);
        }
        case TokenType::STRING_LIT: {
            const Token& t = advance();
            return makeNode(NodeKind::StringLiteral, t.location, t.lexeme);
        }
        default:
            break;
    }

    if (checkLexeme("true") || checkLexeme("false")) {
        const Token& t = advance();
        return makeNode(NodeKind::BoolLiteral, t.location, t.lexeme);
    }

    if (checkLexeme("(")) {
        advance();
        auto inner = parseExpression();
        expectLexeme(")", "expected ')' after expression");
        return inner;
    }

    // Do not consume: the enclosing rule decides how to recover.
    reportError("expected expression", location);
    return makeError(location);
}

std::unique_ptr<Node> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (checkLexeme("(")) {
            advance();
            auto call = makeNode(NodeKind::CallExpr, expr->location);
            call->children.push_back(std::move(expr));

            if (!checkLexeme(")")) {
                do {
                    call->children.push_back(parseExpression());
                } while (matchLexeme(","));
            }
            expectLexeme(")", "expected ')' after arguments");
            expr = std::move(call);
        }
        else if (checkLexeme("[")) {
            advance();
            auto index = makeNode(NodeKind::IndexExpr, expr->location);
            index->children.push_back(std::move(expr));
            index->children.push_back(parseExpression());
            expectLexeme("]", "expected ']' after index");
            expr = std::move(index);
        }
        else if (checkLexeme("++") || checkLexeme("--")) {
            std::string op = advance().lexeme;
            auto postfix = makeNode(NodeKind::PostfixExpr, expr->location, op);
            postfix->children.push_back(std::move(expr));
            expr = std::move(postfix);
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Node> Parser::parseUnary() {
    if (check(TokenType::OPERATOR) && (checkLexeme("!") || checkLexeme("-"))) {
        const Token& t = advance();
        auto node = makeNode(NodeKind::UnaryExpr, t.location, t.lexeme);
        node->children.push_back(parseUnary());
        return node;
    }
    return parsePostfix();
}

// Generic left-associative level:  next { op next }
std::unique_ptr<Node> Parser::parseBinaryLevel(
    ParseFn next, std::initializer_list<const char*> ops) {

    auto left = (this->*next)();

    while (true) {
        bool matched = false;
        if (check(TokenType::OPERATOR)) {
            for (const char* op : ops) {
                if (checkLexeme(op)) {
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) {
            break;
        }

        std::string op = advance().lexeme;
        auto right = (this->*next)();

        auto node = makeNode(NodeKind::BinaryExpr, left->location, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }

    return left;
}

std::unique_ptr<Node> Parser::parsePower() {
    return parseBinaryLevel(&Parser::parseUnary, {"^"});
}

std::unique_ptr<Node> Parser::parseMultiplicative() {
    return parseBinaryLevel(&Parser::parsePower, {"*", "/", "%"});
}

std::unique_ptr<Node> Parser::parseAdditive() {
    return parseBinaryLevel(&Parser::parseMultiplicative, {"+", "-"});
}

std::unique_ptr<Node> Parser::parseRelational() {
    return parseBinaryLevel(&Parser::parseAdditive, {"<", "<=", ">", ">="});
}

std::unique_ptr<Node> Parser::parseEquality() {
    return parseBinaryLevel(&Parser::parseRelational, {"==", "!="});
}

std::unique_ptr<Node> Parser::parseAnd() {
    return parseBinaryLevel(&Parser::parseEquality, {"&&"});
}

std::unique_ptr<Node> Parser::parseOr() {
    return parseBinaryLevel(&Parser::parseAnd, {"||"});
}

std::unique_ptr<Node> Parser::parseAssignment() {
    auto left = parseOr();

    if (check(TokenType::OPERATOR) && checkLexeme("=")) {
        advance();
        auto node = makeNode(NodeKind::AssignExpr, left->location, "=");
        node->children.push_back(std::move(left));
        node->children.push_back(parseAssignment());   // right-associative
        return node;
    }

    return left;
}

std::unique_ptr<Node> Parser::parseExpression() {
    return parseAssignment();
}


// 3. Statements and blocks


std::unique_ptr<Node> Parser::parseReturn() {
    auto node = makeNode(NodeKind::ReturnStmt, advance().location);   // 'return'

    if (!checkLexeme(";")) {
        node->children.push_back(parseExpression());
    }
    if (!expectLexeme(";", "expected ';' after return statement")) {
        synchronize();
    }
    return node;
}

std::unique_ptr<Node> Parser::parsePrint() {
    auto node = makeNode(NodeKind::PrintStmt, advance().location);    // 'print'

    do {
        node->children.push_back(parseExpression());
    } while (matchLexeme(","));

    if (!expectLexeme(";", "expected ';' after print statement")) {
        synchronize();
    }
    return node;
}

std::unique_ptr<Node> Parser::parseIf() {
    auto node = makeNode(NodeKind::IfStmt, advance().location);       // 'if'

    expectLexeme("(", "expected '(' after 'if'");
    node->children.push_back(parseExpression());
    expectLexeme(")", "expected ')' after condition");

    node->children.push_back(parseStatement());

    // Greedy: else binds to the nearest if without an else (dangling else).
    if (matchLexeme("else")) {
        node->children.push_back(parseStatement());
    }
    return node;
}

std::unique_ptr<Node> Parser::parseFor() {
    auto node = makeNode(NodeKind::ForStmt, advance().location);      // 'for'

    expectLexeme("(", "expected '(' after 'for'");

    // initializer
    if (checkLexeme(";")) {
        node->children.push_back(makeNode(NodeKind::EmptyExpr, peek().location));
    } else {
        node->children.push_back(parseExpression());
    }
    expectLexeme(";", "expected ';' after for-initializer");

    // condition
    if (checkLexeme(";")) {
        node->children.push_back(makeNode(NodeKind::EmptyExpr, peek().location));
    } else {
        node->children.push_back(parseExpression());
    }
    expectLexeme(";", "expected ';' after for-condition");

    // increment
    if (checkLexeme(")")) {
        node->children.push_back(makeNode(NodeKind::EmptyExpr, peek().location));
    } else {
        node->children.push_back(parseExpression());
    }
    expectLexeme(")", "expected ')' after for-clauses");

    node->children.push_back(parseStatement());
    return node;
}

std::unique_ptr<Node> Parser::parseStatement() {
    if (checkLexeme("return")) return parseReturn();
    if (checkLexeme("print"))  return parsePrint();
    if (checkLexeme("if"))     return parseIf();
    if (checkLexeme("for"))    return parseFor();
    if (checkLexeme("{"))      return parseBlock();

    // expression statement
    auto node = makeNode(NodeKind::ExprStmt, peek().location);
    node->children.push_back(parseExpression());
    if (!expectLexeme(";", "expected ';' after expression")) {
        synchronize();
    }
    return node;
}

std::unique_ptr<Node> Parser::parseBlockItem() {
    // LL(2): IDENT ':' starts a local declaration, anything else is a statement.
    if (check(TokenType::IDENT) && peek(1).lexeme == ":") {
        return parseDeclaration(false);
    }
    return parseStatement();
}

std::unique_ptr<Node> Parser::parseBlock() {
    const SourceLocation location = peek().location;

    if (!expectLexeme("{", "expected '{' to start a block")) {
        synchronize();
        return makeError(location);
    }

    auto block = makeNode(NodeKind::Block, location);

    while (!checkLexeme("}") && !isAtEnd()) {
        std::size_t before = pos_;
        block->children.push_back(parseBlockItem());

        // Guarantee progress: recovery may stop without consuming anything.
        if (pos_ == before) {
            advance();
        }
    }

    expectLexeme("}", "expected '}' at end of block");
    return block;
}


// 4. Declarations and types


std::unique_ptr<Node> Parser::parseAtomicType() {
    if (check(TokenType::KEYWORD) && isAtomicTypeName(peek().lexeme)) {
        const Token& t = advance();
        return makeNode(NodeKind::TypeNode, t.location, t.lexeme);
    }

    const SourceLocation location = peek().location;
    reportError("expected a type name", location);
    return makeError(location);   // not consumed
}

std::unique_ptr<Node> Parser::parseType() {
    if (!checkLexeme("array")) {
        return parseAtomicType();
    }

    auto node = makeNode(NodeKind::TypeNode, advance().location, "array");

    if (!expectLexeme("[", "expected '[' after 'array'")) {
        return makeError(node->location);
    }

    if (check(TokenType::INT_LIT)) {
        const Token& size = advance();
        node->children.push_back(
            makeNode(NodeKind::IntLiteral, size.location, size.lexeme));
    } else {
        reportError("expected array size", peek().location);
    }

    expectLexeme("]", "expected ']' after array size");
    node->children.push_back(parseAtomicType());
    return node;
}

std::unique_ptr<Node> Parser::parseParameterType() {
    if (!checkLexeme("array")) {
        return parseAtomicType();
    }

    auto node = makeNode(NodeKind::TypeNode, advance().location, "array");

    if (!expectLexeme("[", "expected '[' after 'array'")) {
        return makeError(node->location);
    }
    expectLexeme("]", "expected ']' (array parameters have no size)");

    node->children.push_back(parseAtomicType());
    return node;
}

std::unique_ptr<Node> Parser::parseReturnType() {
    if (checkLexeme("void")) {
        const Token& t = advance();
        return makeNode(NodeKind::TypeNode, t.location, t.lexeme);
    }
    return parseAtomicType();
}

std::unique_ptr<Node> Parser::parseArrayInit() {
    auto node = makeNode(NodeKind::ArrayInit, advance().location);    // '{'

    do {
        node->children.push_back(parseExpression());
    } while (matchLexeme(","));

    expectLexeme("}", "expected '}' at end of array initializer");
    return node;
}

std::unique_ptr<Node> Parser::parseInitializer() {
    if (checkLexeme("{")) {
        return parseArrayInit();
    }
    return parseExpression();
}

std::unique_ptr<Node> Parser::parseVarDeclTail(std::unique_ptr<Node> name) {
    auto node = makeNode(NodeKind::VarDecl, name->location);

    auto type = parseType();
    bool typeFailed = (type->kind == NodeKind::ErrorNode);

    node->children.push_back(std::move(type));
    node->children.push_back(std::move(name));

    if (typeFailed) {
        synchronize();
        return node;
    }

    if (matchLexeme("=")) {
        node->children.push_back(parseInitializer());
    }

    if (!expectLexeme(";", "expected ';' after variable declaration")) {
        synchronize();
    }
    return node;
}

std::unique_ptr<Node> Parser::parseParam() {
    const SourceLocation location = peek().location;

    if (!check(TokenType::IDENT)) {
        reportError("expected parameter name", location);
        return makeError(location);   // not consumed
    }

    const Token& nameTok = advance();
    auto name = makeNode(NodeKind::Identifier, nameTok.location, nameTok.lexeme);

    if (!expectLexeme(":", "expected ':' after parameter name")) {
        return makeError(location);
    }

    auto node = makeNode(NodeKind::Param, location);
    node->children.push_back(parseParameterType());
    node->children.push_back(std::move(name));
    return node;
}

std::unique_ptr<Node> Parser::parseFuncDeclTail(std::unique_ptr<Node> name) {
    auto node = makeNode(NodeKind::FuncDecl, name->location);

    advance();   // 'function'

    node->children.push_back(parseReturnType());
    node->children.push_back(std::move(name));

    expectLexeme("(", "expected '(' after function name");

    if (!checkLexeme(")")) {
        do {
            node->children.push_back(parseParam());
        } while (matchLexeme(","));
    }
    expectLexeme(")", "expected ')' after parameters");

    if (matchLexeme(";")) {
        // declaration only, no body
    }
    else if (matchLexeme("=")) {
        node->children.push_back(parseBlock());
    }
    else {
        reportError("expected ';' or '=' after function header", peek().location);
        synchronize();
    }
    return node;
}

std::unique_ptr<Node> Parser::parseDeclaration(bool allowFunction) {
    const SourceLocation location = peek().location;

    if (!check(TokenType::IDENT)) {
        reportError("expected identifier at start of declaration", location);
        synchronize();
        return makeError(location);
    }

    const Token& nameTok = advance();
    auto name = makeNode(NodeKind::Identifier, nameTok.location, nameTok.lexeme);

    if (!expectLexeme(":", "expected ':' after declaration name")) {
        synchronize();
        return makeError(location);
    }

    if (checkLexeme("function")) {
        if (!allowFunction) {
            // Report, but still parse it so the body is consumed correctly.
            reportError("function declarations are not allowed inside a block",
                        peek().location);
        }
        return parseFuncDeclTail(std::move(name));
    }

    return parseVarDeclTail(std::move(name));
}


// 5. Program and error recovery


std::unique_ptr<Node> Parser::parseProgram() {
    auto program = makeNode(NodeKind::Program, peek().location);

    while (!isAtEnd()) {
        std::size_t before = pos_;
        program->children.push_back(parseDeclaration());

        // Guarantee progress (e.g. a stray '}' or 'if' at top level).
        if (pos_ == before) {
            advance();
        }
    }
    return program;
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (checkLexeme(";")) {
            advance();   // consume the terminator
            return;
        }
        if (checkLexeme("}") || checkLexeme("if") || checkLexeme("for") ||
            checkLexeme("return") || checkLexeme("print")) {
            return;      // do not consume; the caller continues from here
        }
        advance();
    }
}

std::unique_ptr<Node> Parser::parse() {
    return parseProgram();
}