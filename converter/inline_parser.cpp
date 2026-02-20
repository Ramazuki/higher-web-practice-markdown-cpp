#include "inline_parser.h"

#include <string>
#include <vector>

// Довольно типичная задачка для стэка
struct StackEntry {
    std::string marker; // маркер .md
    size_t outputPos; // Позиция, где был найден маркер (в векторе элементов)
    std::string buffer; // текст
};

static void flushBuffer(std::string& buffer, std::vector<InlineElement>& output) {
    if (!buffer.empty()) {
        output.push_back({InlineType::Text, buffer, ""});
        buffer.clear();
    }
}

static std::string collectContent(std::vector<InlineElement>& output, size_t fromPos) {
    std::string result;
    for (size_t j = fromPos; j < output.size(); ++j) {
        result += output[j].content;
    }
    output.erase(output.begin() + static_cast<long>(fromPos), output.end());
    return result;
}

std::vector<InlineElement> parseInline(const std::string& s) {
    std::vector<InlineElement> output;
    std::vector<StackEntry> stack;
    std::string buffer;
    bool escapeActive = false;

    auto handleMarker = [&](const std::string& marker, InlineType type) {
        flushBuffer(buffer, output);
        if (!stack.empty() && stack.back().marker == marker) {
            std::string content = collectContent(output, stack.back().outputPos);
            output.push_back({type, content, ""});
            stack.pop_back();
        } else {
            stack.push_back({marker, output.size(), ""});
        }
    };

    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];

        if (escapeActive) {
            buffer += c;
            escapeActive = false;
            ++i;
            continue;
        }

        if (c == '\\') {
            escapeActive = true;
            ++i;
            continue;
        }

        if (!stack.empty() && stack.back().marker == "`") {
            if (c == '`') {
                handleMarker("`", InlineType::CodeSpan);
            } else {
                buffer += c;
            }
            ++i;
            continue;
        }

        if (c == '`') {
            handleMarker("`", InlineType::CodeSpan);
            ++i;
            continue;
        }

        if (c == '*' && i + 1 < s.size() && s[i + 1] == '*') {
            handleMarker("**", InlineType::Strong);
            i += 2;
            continue;
        }

        if (c == '*') {
            handleMarker("*", InlineType::Emphasis);
            ++i;
            continue;
        }

        if (c == '[') {
            flushBuffer(buffer, output);
            stack.push_back({"[", output.size(), ""});
            ++i;
            continue;
        }

        if (c == ']' && !stack.empty() && stack.back().marker == "[") {
            if (i + 1 < s.size() && s[i + 1] == '(') {
                size_t closeP = s.find(')', i + 2);
                if (closeP != std::string::npos) {
                    flushBuffer(buffer, output);
                    std::string linkText = collectContent(output, stack.back().outputPos);
                    std::string url = s.substr(i + 2, closeP - (i + 2));
                    output.push_back({InlineType::Link, linkText, url});
                    stack.pop_back();
                    i = closeP + 1;
                    continue;
                }
            }
            buffer += c;
            ++i;
            continue;
        }

        buffer += c;
        ++i;
    }

    flushBuffer(buffer, output);

    // Незакрытые маркеры — возвращаем как текст
    while (!stack.empty()) {
        StackEntry entry = stack.back();
        stack.pop_back();
        // вставляем в то место, где изначально нашли
        output.insert(output.begin() + static_cast<long>(entry.outputPos),
                      {InlineType::Text, entry.marker, ""});
        // Сдвигаем outputPos оставшихся записей в стеке
        for (auto& se : stack) {
            if (se.outputPos >= entry.outputPos) {
                ++se.outputPos;
            }
        }
    }

    return output;
}
