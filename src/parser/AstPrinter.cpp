#include "AstPrinter.h"

#include <ostream>

const char* toString(NodeKind kind) {
    switch (kind) {
        // Structural nodes
        case NodeKind::Program:       return "Program";
        case NodeKind::VarDecl:       return "VarDecl";
        case NodeKind::FuncDecl:      return "FuncDecl";
        case NodeKind::Param:         return "Param";
        case NodeKind::Block:         return "Block";

        // Statements
        case NodeKind::ExprStmt:      return "ExprStmt";
        case NodeKind::ReturnStmt:    return "ReturnStmt";
        case NodeKind::PrintStmt:     return "PrintStmt";
        case NodeKind::IfStmt:        return "IfStmt";
        case NodeKind::ForStmt:       return "ForStmt";

        // Expressions
        case NodeKind::AssignExpr:    return "AssignExpr";
        case NodeKind::BinaryExpr:    return "BinaryExpr";
        case NodeKind::UnaryExpr:     return "UnaryExpr";
        case NodeKind::PostfixExpr:   return "PostfixExpr";
        case NodeKind::CallExpr:      return "CallExpr";
        case NodeKind::IndexExpr:     return "IndexExpr";

        // Leaf nodes
        case NodeKind::Identifier:    return "Identifier";
        case NodeKind::IntLiteral:    return "IntLiteral";
        case NodeKind::CharLiteral:   return "CharLiteral";
        case NodeKind::StringLiteral: return "StringLiteral";
        case NodeKind::BoolLiteral:   return "BoolLiteral";

        // Auxiliary nodes
        case NodeKind::ArrayInit:     return "ArrayInit";
        case NodeKind::TypeNode:      return "TypeNode";
        case NodeKind::EmptyExpr:     return "EmptyExpr";
        case NodeKind::ErrorNode:     return "ErrorNode";
    }
    return "Unknown"; // unreachable unless an invalid value is cast to NodeKind
}

void dumpAst(const Node& node, std::ostream& out, int indent) {
    out << std::string(static_cast<std::size_t>(indent) * 2, ' ')
        << toString(node.kind);

    if (!node.text.empty()) {
        out << " \"" << node.text << "\"";
    }

    out << " " << node.location.line << ":" << node.location.col << "\n";

    for (const auto& child : node.children) {
        dumpAst(*child, out, indent + 1);
    }
}