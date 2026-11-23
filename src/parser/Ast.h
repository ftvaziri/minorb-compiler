
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Token.h"

// --------------------------------------------------
// AST Node Kinds
//
// Child ordering conventions:
//
// Program:
//   declarations
//
// VarDecl:
//   TypeNode, Identifier, optional initializer
//
// FuncDecl:
//   return TypeNode, Identifier, parameters..., optional Block
//
// Param:
//   TypeNode, Identifier
//
// Block:
//   declarations and statements
//
// ExprStmt:
//   expression
//
// ReturnStmt:
//   optional expression
//
// PrintStmt:
//   expressions to print
//
// IfStmt:
//   condition, then branch, optional else branch
//
// ForStmt:
//   initializer, condition, increment, body
//   Always exactly 4 children.
//   A missing part is represented by an EmptyExpr node.
//
// AssignExpr:
//   left-hand side, right-hand side
//
// BinaryExpr:
//   left operand, right operand
//
// UnaryExpr:
//   operand
//
// PostfixExpr:
//   operand
//
// CallExpr:
//   callee, arguments...
//
// IndexExpr:
//   array expression, index expression
//
// Identifier, IntLiteral, CharLiteral,
// StringLiteral, BoolLiteral:
//   no children
//
// ArrayInit:
//   initializer expressions
//
// TypeNode:
//   Atomic type:
//     text = type name, no children
//
//   Array type:
//     text = "array"
//     optional IntLiteral child for array size
//     final child is the element TypeNode
//     (array size is omitted for array parameters)
//
// EmptyExpr:
//   no children
//
// ErrorNode:
//   recovered children, if any
// --------------------------------------------------

enum class NodeKind {
    // Structural nodes
    Program,
    VarDecl,
    FuncDecl,
    Param,
    Block,

    // Statements
    ExprStmt,
    ReturnStmt,
    PrintStmt,
    IfStmt,
    ForStmt,

    // Expressions
    AssignExpr,
    BinaryExpr,
    UnaryExpr,
    PostfixExpr,
    CallExpr,
    IndexExpr,

    // Leaf nodes
    Identifier,
    IntLiteral,
    CharLiteral,
    StringLiteral,
    BoolLiteral,

    // Auxiliary nodes
    ArrayInit,
    TypeNode,
    EmptyExpr,
    ErrorNode
};

// Generic AST Node


struct Node {
    NodeKind kind;

    // Source location of the first token belonging to this node.
    SourceLocation location;

    // Meaning depends on NodeKind:
    //
    // Identifier:
    //   Identifier name.
    //
    // IntLiteral, CharLiteral, StringLiteral, BoolLiteral:
    //   Raw lexeme of the literal.
    //
    // BinaryExpr, UnaryExpr, PostfixExpr, AssignExpr:
    //   Operator text (e.g. "+", "!", "++", "=").
    //
    // TypeNode:
    //   Atomic type name, or "array" for array types.
    //
    // Other structural and auxiliary nodes:
    //   Empty string.
    std::string text;

    // Child nodes.
    std::vector<std::unique_ptr<Node>> children;

    // Filled during semantic analysis.
    std::string typeAnnotation;

    Node(
        NodeKind kind,
        SourceLocation location,
        std::string text = ""
    )
        : kind(kind),
          location(std::move(location)),
          text(std::move(text)) {
    }
};


// AST Node Factory


inline std::unique_ptr<Node> makeNode(
    NodeKind kind,
    const SourceLocation& location,
    std::string text = ""
) {
    return std::make_unique<Node>(
        kind,
        location,
        std::move(text)
    );
}
