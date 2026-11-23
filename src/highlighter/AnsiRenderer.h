
#ifndef ANSI_RENDERER_H
#define ANSI_RENDERER_H

#include "Highlighter.h"

#include <string>
#include <vector>

class AnsiRenderer {
public:
    std::string render(
        const std::vector<HighlightedToken>& tokens
    ) const;

private:
    static const std::string& colorCode(HighlightClass category);
};

#endif
