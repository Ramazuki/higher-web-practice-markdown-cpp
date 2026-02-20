#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "utils.h"
#include "scanner.h"
#include "inline_parser.h"
#include "renderer.h"

struct CliArgs {
    std::string inputPath;
    std::string outputPath;
};

void printUsage() {
    std::cerr << "Usage: MarkdownToHTML --in <input.md> [--out <output.html>]\n";
}

CliArgs parseArgs(int argc, char* argv[]) {
    CliArgs args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--in" && i + 1 < argc) {
            args.inputPath = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            args.outputPath = argv[++i];
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage();
            exit(1);
        }
    }

    if (args.inputPath.empty()) {
        std::cerr << "Error: --in <input.md> is required\n";
        printUsage();
        exit(1);
    }

    return args;
}

std::vector<std::string> preprocess(const std::string& raw) {
    std::string normalized = normalizeLineEndings(raw);
    std::vector<std::string> lines = splitLines(normalized);
    for (auto& line : lines) {
        line = trimRight(line);
    }
    return lines;
}

int main(int argc, char* argv[]) {
    CliArgs args = parseArgs(argc, argv);

    std::string raw;
    try {
        raw = readFile(args.inputPath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::vector<std::string> lines = preprocess(raw);
    std::vector<BlockToken> tokens = scan(lines);
    std::string html = renderHtml(tokens);

    if (args.outputPath.empty()) {
        std::cout << html;
    } else {
        std::ofstream out(args.outputPath);
        if (!out.is_open()) {
            std::cerr << "Error: Cannot write to file: " << args.outputPath << "\n";
            return 1;
        }
        out << html;
    }

    return 0;
}
