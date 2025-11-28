#pragma once

#include <unordered_map>
#include <vector>
#include "types.h"

class Parser {
public:
    std::vector<Cell> cells;
    std::vector<Net> nets;
    std::unordered_map<std::string, int> cellNameToIdx;
    int totalSize = 0;
    int offset = 0;
    int cutSize = 0;

    void readInput(const std::string& filename);
    void writeOutput(const std::string& filename, int numPartitions);
};
