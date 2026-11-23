
#ifndef HTML_RENDERER_H
#define HTML_RENDERER_H

#include "Highlighter.h"

#include <string>
#include <vector>

class HtmlRenderer {
public:
    std::string render(
        const std::vector<HighlightedToken>& tokens
    ) const;

private:
    static std::string escapeHtml(const std::string& text);
    static std::string className(HighlightClass category);
    static std::string getCss();
};

#endif
