#pragma once

#include "utils.h"

#include <vector>
#include <string>

enum class BlockType {
    Heading,
    Paragraph,
    OrderedList,
    UnorderedList 
};

struct BlockToken {
    BlockType type;
    int level = 0;
    std::vector<std::string> lines;
};

std::vector<BlockToken> scan(const std::vector<std::string>& lines);
