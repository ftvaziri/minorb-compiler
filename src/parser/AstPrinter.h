#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include <iosfwd>

#include "Ast.h"

// Returns the name of a node kind, e.g. NodeKind::BinaryExpr -> "BinaryExpr".
const char* toString(NodeKind kind);

// Prints the tree rooted at `node`, one node per line:
//   <indent*2 spaces><KindName> ["text"] line:col
void dumpAst(const Node& node, std::ostream& out, int indent = 0);

#endif // AST_PRINTER_H