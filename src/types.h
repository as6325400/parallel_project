#pragma once

#include <string>
#include <vector>

struct Cell {
    std::string name;
    int size;
    int partition;
    int id;
    bool locked;
};

struct Net {
    std::string name;
    int id;
    std::vector<int> cells;
};
