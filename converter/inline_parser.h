#pragma once

#include <string>
#include <vector>

enum class InlineType {
    Text,
    Emphasis,
    Strong,
    CodeSpan,
    Link 
};

struct InlineElement {
    InlineType type;
    std::string content;
    std::string url;
};

std::vector<InlineElement> parseInline(const std::string& input);
