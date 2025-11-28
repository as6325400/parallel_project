#include "parser.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void Parser::readInput(const string& filename) {
    ifstream inFile(filename);
    string keyword;
    int numCells, numNets;

    inFile >> keyword >> numCells;
    cells.resize(numCells);

    for (int i = 0; i < numCells; i++) {
        inFile >> keyword >> cells[i].name >> cells[i].size;
        cellNameToIdx[cells[i].name] = i;
        cells[i].id = i;
        totalSize += cells[i].size;
        cells[i].partition = -1;
        cells[i].locked = false;
    }

    inFile >> keyword >> numNets;
    nets.resize(numNets);
    vector<int> countOffset(numCells);

    for (int i = 0; i < numNets; i++) {
        int numCellsInNet;
        inFile >> keyword >> nets[i].name >> numCellsInNet;
        nets[i].cells.resize(numCellsInNet);
        for (int j = 0; j < numCellsInNet; j++) {
            string cellName;
            inFile >> keyword >> cellName;
            int cellIdx = cellNameToIdx[cellName];
            nets[i].cells[j] = cellIdx;
            nets[i].id = i;
            countOffset[cellIdx]++;
        }
    }

    offset = *max_element(countOffset.begin(), countOffset.end());
    inFile.close();
}

void Parser::writeOutput(const string& filename, int numPartitions) {
    ofstream outFile(filename);
    outFile << "CutSize " << cutSize << "\n";

    vector<vector<string>> groups(numPartitions);
    for (const auto& cell : cells) {
        groups[cell.partition].push_back(cell.name);
    }

    string groupNames[] = {"GroupA", "GroupB", "GroupC", "GroupD"};
    for (int i = 0; i < numPartitions; i++) {
        outFile << groupNames[i] << " " << groups[i].size() << "\n";
        for (const auto& cellName : groups[i]) {
            outFile << cellName << "\n";
        }
    }
    outFile.close();
}
