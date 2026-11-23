#include "HtmlRenderer.h"

std::string HtmlRenderer::escapeHtml(const std::string& text) {
    std::string result;

    for (char ch : text) {
        switch (ch) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&#39;";  break;
            default:   result += ch;      break;
        }
    }

    return result;
}

std::string HtmlRenderer::className(HighlightClass category) {
    switch (category) {
        case HighlightClass::Keyword:   return "keyword";
        case HighlightClass::Type:      return "type";
        case HighlightClass::Function:  return "function";
        case HighlightClass::Variable:  return "variable";
        case HighlightClass::Literal:   return "literal";
        case HighlightClass::Operator:  return "operator";
        case HighlightClass::Delimiter: return "delimiter";
        case HighlightClass::Comment:   return "comment";
        case HighlightClass::Error:     return "error";
        case HighlightClass::Plain:     return "";
    }

    return "";
}

std::string HtmlRenderer::getCss() {
    return R"(
        body {
            background: #1e1e1e;
            color: #d4d4d4;
            padding: 20px;
        }

        pre {
            font-family: Menlo, Consolas, "DejaVu Sans Mono", monospace;
            font-size: 14px;
            line-height: 1.6;
            white-space: pre;
            overflow-x: auto;
        }

        .keyword   { color: #569cd6; font-weight: bold; }
        .type      { color: #4ec9b0; font-weight: bold; }
        .function  { color: #dcdcaa; }
        .variable  { color: #d4d4d4; }
        .literal   { color: #ce9178; }
        .operator  { color: #c586c0; }
        .delimiter { color: #808080; }
        .comment   { color: #6a9955; }
        .error     { color: #f44747; text-decoration: underline wavy; }
    )";
}

std::string HtmlRenderer::render(
    const std::vector<HighlightedToken>& tokens
) const {
    std::string output;

    output += "<!DOCTYPE html>\n";
    output += "<html lang=\"en\">\n";
    output += "<head>\n";
    output += "<meta charset=\"UTF-8\">\n";
    output += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    output += "<title>Minor-B Syntax Highlighting</title>\n";
    output += "<style>\n";
    output += getCss();
    output += "\n</style>\n";
    output += "</head>\n";
    output += "<body>\n";
    output += "<pre><code>";

    for (const auto& item : tokens) {
        std::string text = escapeHtml(item.token.lexeme);
        std::string cssClass = className(item.category);

        if (cssClass.empty()) {
            output += text;
        } else {
            output += "<span class=\"" + cssClass + "\">";
            output += text;
            output += "</span>";
        }
    }

    output += "</code></pre>\n";
    output += "</body>\n";
    output += "</html>\n";

    return output;
}