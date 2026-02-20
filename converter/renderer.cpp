#include "renderer.h"
#include "inline_parser.h"
#include "utils.h"

static std::string renderInline(const std::string& text) {
    std::string result;
    auto elements = parseInline(text);
    for (const auto& el : elements) {
        switch (el.type) {
            case InlineType::Text:
                result += escapeHtml(el.content);
                break;
            case InlineType::Emphasis:
                result += "<em>" + escapeHtml(el.content) + "</em>";
                break;
            case InlineType::Strong:
                result += "<strong>" + escapeHtml(el.content) + "</strong>";
                break;
            case InlineType::CodeSpan:
                result += "<code>" + escapeHtml(el.content) + "</code>";
                break;
            case InlineType::Link:
                result += "<a href=\"" + escapeHtml(el.url) + "\">"
                        + escapeHtml(el.content) + "</a>";
                break;
        }
    }
    return result;
}

std::string renderHtml(const std::vector<BlockToken>& tokens) {
    std::string html;
    for (const auto& token : tokens) {
        switch (token.type) {
            case BlockType::Heading:
                html += "<h" + std::to_string(token.level) + ">"
                      + renderInline(token.lines[0])
                      + "</h" + std::to_string(token.level) + ">\n";
                break;
            case BlockType::Paragraph:
                html += "<p>" + renderInline(token.lines[0]) + "</p>\n";
                break;
            case BlockType::OrderedList:
                html += "<ol>\n";
                for (const auto& item : token.lines) {
                    html += "  <li>" + renderInline(item) + "</li>\n";
                }
                html += "</ol>\n";
                break;
            case BlockType::UnorderedList:
                html += "<ul>\n";
                for (const auto& item : token.lines) {
                    html += "  <li>" + renderInline(item) + "</li>\n";
                }
                html += "</ul>\n";
                break;
        }
    }
    return html;
}
