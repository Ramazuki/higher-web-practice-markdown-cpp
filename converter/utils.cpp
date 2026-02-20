#include "utils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string normalizeLineEndings(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            result += '\n';
            ++i;
        } else {
            result += text[i];
        }
    }
    return result;
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string trimRight(const std::string& line) {
    auto end = line.find_last_not_of(" \t\r\n");
    if (end == std::string::npos) {
        return "";
    }
    return line.substr(0, end + 1);
}

std::string escapeHtml(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;";  break;
            case '<': result += "&lt;";   break;
            case '>': result += "&gt;";   break;
            case '"': result += "&quot;"; break;
            default:  result += c;        break;
        }
    }
    return result;
}
