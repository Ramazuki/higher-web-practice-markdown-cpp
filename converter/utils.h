#pragma once

#include <string>
#include <vector>

std::string readFile(const std::string& path);
std::string normalizeLineEndings(const std::string& text);
std::vector<std::string> splitLines(const std::string& text);
std::string trimRight(const std::string& line);
std::string escapeHtml(const std::string& text);
