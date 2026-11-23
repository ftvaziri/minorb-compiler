#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "src/lexer/Lexer.h"
#include "src/parser/Ast.h"
#include "src/parser/AstPrinter.h"

void runTest(const std::string& source) {
    Lexer lexer(source, "test.bminor");
    lexer.tokenize();

    for (const Diagnostic& diagnostic : lexer.diagnostics()) {
        std::cout << diagnostic.location.filename
                  << ":" << diagnostic.location.line
                  << ":" << diagnostic.location.col
                  << ": error: " << diagnostic.message
                  << "\n";
    }
}

static SourceLocation loc(int line, int col) {
    SourceLocation l;
    l.filename = "test.bminor";
    l.line = line;
    l.col = col;
    return l;
}

// Builds the tree for:  1 + 2 * 3
void runAstDumpTest() {
    auto mul = makeNode(NodeKind::BinaryExpr, loc(1, 5), "*");
    mul->children.push_back(makeNode(NodeKind::IntLiteral, loc(1, 5), "2"));
    mul->children.push_back(makeNode(NodeKind::IntLiteral, loc(1, 9), "3"));

    auto add = makeNode(NodeKind::BinaryExpr, loc(1, 1), "+");
    add->children.push_back(makeNode(NodeKind::IntLiteral, loc(1, 1), "1"));
    add->children.push_back(std::move(mul));

    dumpAst(*add, std::cout);
}

int main() {
    runTest("x = 5 @ 3");
    runTest("\"abc");
    runTest("/* x");
    runTest("'''");
    runTest("integer age = 25;");

    std::cout << "--- AST dump ---\n";
    runAstDumpTest();

    return 0;
}