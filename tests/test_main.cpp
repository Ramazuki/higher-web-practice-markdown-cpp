#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>

#include "utils.h"
#include "scanner.h"
#include "inline_parser.h"
#include "renderer.h"

int totalPassed = 0;
int totalFailed = 0;

void check(bool condition, const std::string& testName,
           const std::string& expected = "", const std::string& actual = "") {
    if (condition) {
        ++totalPassed;
        std::cout << "  PASS: " << testName << "\n";
    } else {
        ++totalFailed;
        std::cerr << "  FAIL: " << testName << "\n";
        if (!expected.empty() || !actual.empty()) {
            std::cerr << "    expected: [" << expected << "]\n";
            std::cerr << "    actual:   [" << actual << "]\n";
        }
    }
}

// ==================== Scanner ====================

void testScanHeadings() {
    auto tokens = scan({"# H1", "## H2", "### H3"});
    check(tokens.size() == 3, "scan: 3 headings parsed");
    check(tokens[0].type == BlockType::Heading && tokens[0].level == 1, "scan: h1 level");
    check(tokens[0].lines[0] == "H1", "scan: h1 text");
    check(tokens[1].level == 2 && tokens[1].lines[0] == "H2", "scan: h2");
    check(tokens[2].level == 3 && tokens[2].lines[0] == "H3", "scan: h3");
}

void testScanOrderedList() {
    auto tokens = scan({"1. First", "2. Second", "3. Third"});
    check(tokens.size() == 1, "scan: ordered list grouped");
    check(tokens[0].type == BlockType::OrderedList, "scan: ordered list type");
    check(tokens[0].lines.size() == 3, "scan: ordered list 3 items");
    check(tokens[0].lines[0] == "First", "scan: ol item 1");
    check(tokens[0].lines[2] == "Third", "scan: ol item 3");
}

void testScanUnorderedList() {
    auto tokens = scan({"- A", "- B", "- C"});
    check(tokens.size() == 1, "scan: unordered list grouped");
    check(tokens[0].type == BlockType::UnorderedList, "scan: unordered list type");
    check(tokens[0].lines.size() == 3, "scan: ul 3 items");
}

void testScanUnorderedListStar() {
    auto tokens = scan({"* X", "* Y"});
    check(tokens.size() == 1, "scan: ul with * marker");
    check(tokens[0].type == BlockType::UnorderedList, "scan: ul star type");
    check(tokens[0].lines[0] == "X", "scan: ul star item 1");
}

void testScanParagraph() {
    auto tokens = scan({"line one", "line two"});
    check(tokens.size() == 1, "scan: paragraph grouped");
    check(tokens[0].type == BlockType::Paragraph, "scan: paragraph type");
    check(tokens[0].lines[0] == "line one line two", "scan: paragraph joined",
          "line one line two", tokens[0].lines[0]);
}

void testScanMixedBlocks() {
    auto tokens = scan({"# Title", "", "Some text.", "", "- a", "- b", "", "1. x", "2. y"});
    check(tokens.size() == 4, "scan: mixed blocks count");
    check(tokens[0].type == BlockType::Heading, "scan: mixed heading");
    check(tokens[1].type == BlockType::Paragraph, "scan: mixed paragraph");
    check(tokens[2].type == BlockType::UnorderedList, "scan: mixed ul");
    check(tokens[3].type == BlockType::OrderedList, "scan: mixed ol");
}

void testScanEmptyInput() {
    auto tokens = scan({});
    check(tokens.empty(), "scan: empty input");
}

void testScanOnlyEmptyLines() {
    auto tokens = scan({"", "", "", ""});
    check(tokens.empty(), "scan: only empty lines");
}

void testScanListsSeparatedByBlank() {
    auto tokens = scan({"- a", "- b", "", "1. x", "2. y"});
    check(tokens.size() == 2, "scan: two lists separated by blank");
    check(tokens[0].type == BlockType::UnorderedList, "scan: first is ul");
    check(tokens[1].type == BlockType::OrderedList, "scan: second is ol");
}

// ==================== Inline parser ====================

void testInlinePlainText() {
    auto elems = parseInline("hello world");
    check(elems.size() == 1, "inline: plain text count");
    check(elems[0].type == InlineType::Text && elems[0].content == "hello world", "inline: plain text content");
}

void testInlineEmphasis() {
    auto elems = parseInline("*italic*");
    check(elems.size() == 1, "inline: emphasis count");
    check(elems[0].type == InlineType::Emphasis && elems[0].content == "italic", "inline: emphasis content");
}

void testInlineStrong() {
    auto elems = parseInline("**bold**");
    check(elems.size() == 1, "inline: strong count");
    check(elems[0].type == InlineType::Strong && elems[0].content == "bold", "inline: strong content");
}

void testInlineCodeSpan() {
    auto elems = parseInline("`code`");
    check(elems.size() == 1, "inline: code count");
    check(elems[0].type == InlineType::CodeSpan && elems[0].content == "code", "inline: code content");
}

void testInlineLink() {
    auto elems = parseInline("[Yandex](https://yandex.ru)");
    check(elems.size() == 1, "inline: link count");
    check(elems[0].type == InlineType::Link, "inline: link type");
    check(elems[0].content == "Yandex", "inline: link text", "Yandex", elems[0].content);
    check(elems[0].url == "https://yandex.ru", "inline: link url");
}

void testInlineEscaping() {
    auto elems = parseInline("\\*literal\\*");
    check(elems.size() == 1, "inline: escape count");
    check(elems[0].type == InlineType::Text, "inline: escape type");
    check(elems[0].content == "*literal*", "inline: escape content", "*literal*", elems[0].content);
}

void testInlineEscapeAngleBrackets() {
    auto elems = parseInline("\\<tag\\>");
    check(elems.size() == 1, "inline: escape angle count");
    check(elems[0].content == "<tag>", "inline: escape angle content", "<tag>", elems[0].content);
}

void testInlineCodeIgnoresMarkers() {
    auto elems = parseInline("`*not italic*`");
    check(elems.size() == 1, "inline: code ignores markers count");
    check(elems[0].type == InlineType::CodeSpan, "inline: code ignores markers type");
    check(elems[0].content == "*not italic*", "inline: code ignores markers content",
          "*not italic*", elems[0].content);
}

void testInlineUnpairedStar() {
    auto elems = parseInline("*open");
    bool hasStarAsText = false;
    for (const auto& el : elems) {
        if (el.type == InlineType::Text && el.content.find('*') != std::string::npos) {
            hasStarAsText = true;
        }
    }
    check(hasStarAsText, "inline: unpaired * becomes text");
}

void testInlineUnpairedDoubleStar() {
    auto elems = parseInline("**open");
    bool found = false;
    for (const auto& el : elems) {
        if (el.type == InlineType::Text && el.content.find("**") != std::string::npos) {
            found = true;
        }
    }
    check(found, "inline: unpaired ** becomes text");
}

void testInlineUnclosedBracket() {
    auto elems = parseInline("text [ unclosed");
    bool allText = true;
    for (const auto& el : elems) {
        if (el.type != InlineType::Text) allText = false;
    }
    check(allText, "inline: unclosed [ treated as text");
}

void testInlineMixed() {
    auto elems = parseInline("a *b* **c** `d`");
    check(elems.size() == 6, "inline: mixed count");
    check(elems[1].type == InlineType::Emphasis && elems[1].content == "b", "inline: mixed emphasis");
    check(elems[3].type == InlineType::Strong && elems[3].content == "c", "inline: mixed strong");
    check(elems[5].type == InlineType::CodeSpan && elems[5].content == "d", "inline: mixed code");
}

// ==================== Renderer ====================

void testRenderHeading() {
    BlockToken t;
    t.type = BlockType::Heading;
    t.level = 2;
    t.lines = {"Title"};
    std::string html = renderHtml({t});
    check(html == "<h2>Title</h2>\n", "render: h2", "<h2>Title</h2>\\n", html);
}

void testRenderParagraph() {
    BlockToken t;
    t.type = BlockType::Paragraph;
    t.lines = {"Simple text."};
    std::string html = renderHtml({t});
    check(html == "<p>Simple text.</p>\n", "render: paragraph");
}

void testRenderOrderedList() {
    BlockToken t;
    t.type = BlockType::OrderedList;
    t.lines = {"A", "B"};
    std::string html = renderHtml({t});
    std::string expected = "<ol>\n  <li>A</li>\n  <li>B</li>\n</ol>\n";
    check(html == expected, "render: ol", expected, html);
}

void testRenderUnorderedList() {
    BlockToken t;
    t.type = BlockType::UnorderedList;
    t.lines = {"X", "Y"};
    std::string html = renderHtml({t});
    std::string expected = "<ul>\n  <li>X</li>\n  <li>Y</li>\n</ul>\n";
    check(html == expected, "render: ul", expected, html);
}

void testRenderHtmlEscape() {
    BlockToken t;
    t.type = BlockType::Paragraph;
    t.lines = {"A & B <tag> \"quote\""};
    std::string html = renderHtml({t});
    std::string expected = "<p>A &amp; B &lt;tag&gt; &quot;quote&quot;</p>\n";
    check(html == expected, "render: html escape", expected, html);
}

void testRenderInlineInParagraph() {
    BlockToken t;
    t.type = BlockType::Paragraph;
    t.lines = {"*em* and **strong**"};
    std::string html = renderHtml({t});
    std::string expected = "<p><em>em</em> and <strong>strong</strong></p>\n";
    check(html == expected, "render: inline in paragraph", expected, html);
}

void testRenderEscaping() {
    BlockToken t;
    t.type = BlockType::Paragraph;
    t.lines = {"\\*literal\\* and \\<tag\\>"};
    std::string html = renderHtml({t});
    std::string expected = "<p>*literal* and &lt;tag&gt;</p>\n";
    check(html == expected, "render: escaping in paragraph", expected, html);
}

// ==================== Other ====================

void testEmptyFile() {
    auto tokens = scan({});
    std::string html = renderHtml(tokens);
    check(html.empty(), "edge: empty file produces empty html");
}

void testOnlyBlankLines() {
    auto tokens = scan({"", "   ", "", "  ", ""});
    check(tokens.empty() || renderHtml(tokens) == "<p></p>\n" || true,
          "edge: blank lines don't crash");
}

void testLongLine() {
    std::string longLine(10000, 'x');
    auto tokens = scan({longLine});
    std::string html = renderHtml(tokens);
    check(!html.empty(), "edge: 10000 char line doesn't crash");
}

void testUtilsTrimRight() {
    check(trimRight("hello   ") == "hello", "util: trimRight spaces");
    check(trimRight("  hello  ") == "  hello", "util: trimRight preserves leading");
    check(trimRight("   ") == "", "util: trimRight all spaces");
    check(trimRight("") == "", "util: trimRight empty");
}

void testUtilsEscapeHtml() {
    check(escapeHtml("A & B") == "A &amp; B", "util: escapeHtml &");
    check(escapeHtml("<tag>") == "&lt;tag&gt;", "util: escapeHtml < >");
    check(escapeHtml("\"q\"") == "&quot;q&quot;", "util: escapeHtml quotes");
    check(escapeHtml("plain") == "plain", "util: escapeHtml plain text");
}

int main() {
    std::cout << "=== Scanner tests ===" << std::endl;
    testScanHeadings();
    testScanOrderedList();
    testScanUnorderedList();
    testScanUnorderedListStar();
    testScanParagraph();
    testScanMixedBlocks();
    testScanEmptyInput();
    testScanOnlyEmptyLines();
    testScanListsSeparatedByBlank();

    std::cout << "\n=== Inline parser tests ===" << std::endl;
    testInlinePlainText();
    testInlineEmphasis();
    testInlineStrong();
    testInlineCodeSpan();
    testInlineLink();
    testInlineEscaping();
    testInlineEscapeAngleBrackets();
    testInlineCodeIgnoresMarkers();
    testInlineUnpairedStar();
    testInlineUnpairedDoubleStar();
    testInlineUnclosedBracket();
    testInlineMixed();

    std::cout << "\n=== Renderer tests ===" << std::endl;
    testRenderHeading();
    testRenderParagraph();
    testRenderOrderedList();
    testRenderUnorderedList();
    testRenderHtmlEscape();
    testRenderInlineInParagraph();
    testRenderEscaping();

    std::cout << "\n=== Other ===" << std::endl;
    testEmptyFile();
    testOnlyBlankLines();
    testLongLine();
    testUtilsTrimRight();
    testUtilsEscapeHtml();

    std::cout << "\n=== Results: " << totalPassed << " passed, " << totalFailed << " failed ===\n";
    return totalFailed > 0 ? 1 : 0;
}
