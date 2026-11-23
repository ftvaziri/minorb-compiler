#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "src/lexer/Lexer.h"
#include "src/lexer/Diagnostic.h"
#include "src/parser/Ast.h"
#include "src/parser/AstPrinter.h"
#include "src/parser/Parser.h"


// چاپ Diagnosticها
void printDiagnostics(
    const std::string& title,
    const std::vector<Diagnostic>& diagnostics
) {
    std::cout << "\n--- " << title << " ---\n";

    if (diagnostics.empty()) {
        std::cout << "No diagnostics.\n";
        return;
    }

    for (const auto& diagnostic : diagnostics) {
        std::cout
            << "[" << diagnostic.location.line
            << ":" << diagnostic.location.col << "] "
            << diagnostic.message << "\n";
    }
}

// اجرای Lexer و Parser و چاپ نتیجه
void runParserTest(
    const std::string& testName,
    const std::string& source
) {
    std::cout << "\n\n========================================\n";
    std::cout << "TEST: " << testName << "\n";
    std::cout << "========================================\n";

    // Lexer
    Lexer lexer(source, "test.bminor");
    std::vector<Token> tokens = lexer.tokenize();

    // Parser
    Parser parser(std::move(tokens));
    std::unique_ptr<Node> ast = parser.parse();

    // AST
    std::cout << "\n--- AST ---\n";

    if (ast) {
        dumpAst(*ast, std::cout);
    } else {
        std::cout << "AST is null.\n";
    }

    // Diagnostics
    printDiagnostics("Lexer Diagnostics", lexer.diagnostics());
    printDiagnostics("Parser Diagnostics", parser.diagnostics());
}

int main() {
    // 1. برنامه معتبر
    const std::string validProgram = R"(
square: function integer (x: integer) = {
    return x ^ 2;
}

main: function integer () = {
    x: integer = 10;

    if (x > 5) {
        print square(x);
    } else {
        print 0;
    }

    return 0;
}
)";


    // 2. Dangling Else
    const std::string danglingElse = R"(
main: function integer () = {
    if (true)
        if (false)
            print 1;
        else
            print 2;

    return 0;
}
)";


    // 3. جا افتادن Semicolon
    const std::string missingSemicolon = R"(
main: function integer () = {
    x: integer = 10
    return 0;
}
)";


    // 4. جا افتادن آکولاد بسته
    const std::string missingBrace = R"(
main: function integer () = {
    return 0;
)";


    // 5. تعریف تابع داخل یک Block
    const std::string nestedFunction = R"(
main: function integer () = {
    inner: function integer () = {
        return 1;
    }

    return 0;
}
)";


    // 6. ورودی خالی
    const std::string emptyInput = "";


    // اجرای تست‌ها
    runParserTest("Valid Program", validProgram);

    runParserTest("Dangling Else", danglingElse);

    runParserTest("Missing Semicolon", missingSemicolon);

    runParserTest("Missing Closing Brace", missingBrace);

    runParserTest("Function Inside Block", nestedFunction);

    runParserTest("Empty Input", emptyInput);


    return 0;
}