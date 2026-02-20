#include "scanner.h"

#include <regex>
#include <functional>

struct ScanContext {
    const std::vector<std::string>& lines;
    size_t& i;
    std::smatch& match;
    const std::regex& headingRe;
    const std::regex& orderedRe;
    const std::regex& unorderedRe;
};

// Будем использовать спецификации для парса блоков
template<BlockType Type>
BlockToken parseBlock(ScanContext& ctx);

template<>
BlockToken parseBlock<BlockType::Heading>(ScanContext& ctx) {
    BlockToken token;
    token.type = BlockType::Heading;
    token.level = static_cast<int>(ctx.match[1].str().size());
    token.lines.push_back(ctx.match[2].str());
    ++ctx.i;
    return token;
}

template<>
BlockToken parseBlock<BlockType::OrderedList>(ScanContext& ctx) {
    BlockToken token;
    token.type = BlockType::OrderedList;
    while (ctx.i < ctx.lines.size() && std::regex_match(ctx.lines[ctx.i], ctx.match, ctx.orderedRe)) {
        token.lines.push_back(ctx.match[1].str());
        ++ctx.i;
    }
    return token;
}

template<>
BlockToken parseBlock<BlockType::UnorderedList>(ScanContext& ctx) {
    BlockToken token;
    token.type = BlockType::UnorderedList;
    while (ctx.i < ctx.lines.size() && std::regex_match(ctx.lines[ctx.i], ctx.match, ctx.unorderedRe)) {
        token.lines.push_back(ctx.match[1].str());
        ++ctx.i;
    }
    return token;
}

template<>
BlockToken parseBlock<BlockType::Paragraph>(ScanContext& ctx) {
    BlockToken token;
    token.type = BlockType::Paragraph;
    std::string paragraph;
    auto isBlockStart = [&](const std::string& line) -> bool {
        return std::regex_match(line, ctx.headingRe)
            || std::regex_match(line, ctx.orderedRe)
            || std::regex_match(line, ctx.unorderedRe);
    };
    while (ctx.i < ctx.lines.size() && !ctx.lines[ctx.i].empty() && !isBlockStart(ctx.lines[ctx.i])) {
        if (!paragraph.empty()) paragraph += " ";
        paragraph += ctx.lines[ctx.i];
        ++ctx.i;
    }
    token.lines.push_back(std::move(paragraph));
    return token;
}

std::vector<BlockToken> scan(const std::vector<std::string>& lines) {
    std::vector<BlockToken> tokens;

    // При помощи регулярных выражений находим нужные блоки и отделяем в них текст
    std::regex headingRe(R"(^(#{1,3})\s+(.*)$)");
    std::regex orderedRe(R"(^[0-9]+\.\s+(.*)$)");
    std::regex unorderedRe(R"(^[-*]\s+(.*)$)");

    size_t i = 0;
    std::smatch match;
    ScanContext ctx{lines, i, match, headingRe, orderedRe, unorderedRe};

    while (i < lines.size()) {
        if (lines[i].empty()) {
            ++i;
            continue;
        }

        if (std::regex_match(lines[i], match, headingRe)) {
            tokens.push_back(parseBlock<BlockType::Heading>(ctx));
        } else if (std::regex_match(lines[i], match, orderedRe)) {
            tokens.push_back(parseBlock<BlockType::OrderedList>(ctx));
        } else if (std::regex_match(lines[i], match, unorderedRe)) {
            tokens.push_back(parseBlock<BlockType::UnorderedList>(ctx));
        } else {
            tokens.push_back(parseBlock<BlockType::Paragraph>(ctx));
        }
    }

    return tokens;
}
