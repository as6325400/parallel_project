#pragma once

#include <array>
#include <utility>
#include <vector>

#include "parser.h"

class FMPartitioner {
public:
    FMPartitioner(Parser& p, double lowerRatio, double upperRatio);
    void partition(const std::vector<int>& init, int k, bool isParent);

private:
    struct BucketNode {
        int prev = -1;
        int next = -1;
    };

    Parser& parser;
    std::vector<int> partitionSizes;
    int minSize, maxSize;
    int K;
    std::vector<std::vector<int>> cell2nets;
    std::vector<std::array<int,2>> netCount;
    std::vector<int> cellGain;
    double lr, rr;
    int offset;
    std::vector<char> touched;
    std::vector<int> delta;
    
    std::vector<int> buckets;
    std::vector<BucketNode> bucketNodes;
    std::vector<char> inBucket;

    int computeCutSize();
    void removeFromBucket(int cellIdx);
    void addToBucket(int cellIdx, int gain);
    int computeGain(int cellIdx);
    bool canMove(int cellIdx);
    void initBuckets();
    void updateGains(int movedCell);
    std::pair<int, int> findBestMove();
    void makeMove(int cellIdx);
    int fmPass();
    std::vector<int> greedyInit2Way(const Parser& P);
    void buildSubParser(const std::vector<int>& subset, Parser& sub, std::vector<int>& new2old);
    void mergeBack(int whichSide, const Parser& sub, const std::vector<int>& new2old);
};

std::vector<int> makeGreedyRandomInit(const Parser& parser);
