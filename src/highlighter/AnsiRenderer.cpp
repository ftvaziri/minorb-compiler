
#include "AnsiRenderer.h"

#include <unordered_map>

const std::string& AnsiRenderer::colorCode(HighlightClass category) {
    static const std::unordered_map<HighlightClass, std::string> colors = {
        {HighlightClass::Keyword,   "\033[1;34m"}, // Bold Blue
        {HighlightClass::Type,      "\033[1;36m"}, // Bold Cyan
        {HighlightClass::Function,  "\033[32m"},   // Green
        {HighlightClass::Variable,  "\033[37m"},   // White
        {HighlightClass::Literal,   "\033[33m"},   // Yellow
        {HighlightClass::Operator,  "\033[35m"},   // Magenta
        {HighlightClass::Delimiter, "\033[90m"},   // Gray
        {HighlightClass::Comment,   "\033[90m"},   // Gray
        {HighlightClass::Error,     "\033[1;31m"}, // Bold Red
        {HighlightClass::Plain,     ""}
    };

    static const std::string empty = "";

    auto it = colors.find(category);

    if (it != colors.end()) {
        return it->second;
    }

    return empty;
}

std::string AnsiRenderer::render(
    const std::vector<HighlightedToken>& tokens
) const {
    std::string output;

    for (const auto& item : tokens) {
        const std::string& color = colorCode(item.category);

        if (color.empty()) {
            output += item.token.lexeme;
        } else {
            output += color;
            output += item.token.lexeme;
            output += "\033[0m";
        }
    }

    return output;
}
