#include "fm_partitioner.h"

#include <algorithm>
#include <random>
#include <unordered_map>

using namespace std;

FMPartitioner::FMPartitioner(Parser& p, double lowerRatio, double upperRatio) : parser(p) {
    partitionSizes.resize(2, 0);
    lr = lowerRatio;
    rr = upperRatio;
    minSize = (int)ceil(p.totalSize * lr);
    maxSize = (int)floor(p.totalSize * rr);
    offset = p.offset;
    buckets.resize(2 * offset + 1);
    bucketNodes.resize(p.cells.size());
    cell2nets.resize(p.cells.size());
    inBucket.assign(p.cells.size(), 0);

    for (int i = 0; i < (int)p.nets.size(); i++){
        for (auto cellIdx : p.nets[i].cells) {
            cell2nets[cellIdx].push_back(i);
        }
    }
}

int FMPartitioner::computeCutSize() {
    int cut = 0;
    for (const auto& net : parser.nets) {
        int firstP = parser.cells[net.cells[0]].partition;
        for (int cellIdx : net.cells) {
            if (parser.cells[cellIdx].partition != firstP) {
                cut++;
                break;
            }
        }
    }
    return cut;
}

void FMPartitioner::removeFromBucket(int cellIdx) {
    int gain = cellGain[cellIdx];
    int bucketIdx = gain + offset;

    const auto& node = bucketNodes[cellIdx];
    int prev = node.prev;
    int next = node.next;

    if (prev != -1) {
        bucketNodes[prev].next = next;
    } else {
        buckets[bucketIdx] = next;
    }

    if (next != -1) {
        bucketNodes[next].prev = prev;
    }

    bucketNodes[cellIdx].prev = -1;
    bucketNodes[cellIdx].next = -1;
    inBucket[cellIdx] = 0;
}

void FMPartitioner::addToBucket(int cellIdx, int gain) {
    int bucketIdx = gain + offset;
    int head = buckets[bucketIdx];

    bucketNodes[cellIdx].next = head;
    bucketNodes[cellIdx].prev = -1;

    if (head != -1) {
        bucketNodes[head].prev = cellIdx;
    }
    buckets[bucketIdx] = cellIdx;
    inBucket[cellIdx] = 1;
}

int FMPartitioner::computeGain(int cellIdx) {
    int fromP = parser.cells[cellIdx].partition;
    int toP = 1 - fromP;
    int gain = 0;
    for (int ni : cell2nets[cellIdx]) {
        int F = netCount[ni][fromP];
        int T = netCount[ni][toP];
        if (F == 1) gain++;
        else if (T == 0) gain--;
    }
    return gain;
}

bool FMPartitioner::canMove(int cellIdx) {
    int fromP = parser.cells[cellIdx].partition;
    int toP = 1 - fromP;
    int cellSize = parser.cells[cellIdx].size;
    int newFromSize = partitionSizes[fromP] - cellSize;
    int newToSize = partitionSizes[toP] + cellSize;
    return (newFromSize >= minSize && newFromSize <= maxSize && newToSize >= minSize && newToSize <= maxSize);
}

void FMPartitioner::initBuckets() {
    mt19937 rng(random_device{}());
    auto &cells = parser.cells;
    auto &nets = parser.nets;

    for (int i = 0; i < (int)cells.size(); i++) cells[i].locked = false;
    fill(buckets.begin(), buckets.end(), -1);

    netCount.assign(nets.size(), {0, 0});
    cellGain.assign(cells.size(), 0);
    inBucket.assign(cells.size(), 0);

    for(int i = 0; i < (int)nets.size(); i++) {
        for(auto cellIdx : parser.nets[i].cells ) {
            netCount[i][cells[cellIdx].partition]++;
        }
    }

    vector<int> indices(cells.size());
    for(int i = 0; i < (int)cells.size(); i++) indices[i] = i;
    shuffle(indices.begin(), indices.end(), rng);

    for (int i : indices) {
        int gain = computeGain(i);
        cellGain[i] = gain;
        addToBucket(i, gain);
    }
}

void FMPartitioner::updateGains(int movedCell) {
    int toP = parser.cells[movedCell].partition;
    int fromP = 1 - toP;

    const int N = (int)parser.cells.size();
    if ((int)touched.size() != N) {
        touched.assign(N, 0);
        delta.assign(N, 0);
    }

    vector<int> affected;
    affected.reserve(64);

    for (int e : cell2nets[movedCell]) {
        int F_after = netCount[e][fromP];
        int T_after = netCount[e][toP];
        int F_before = F_after + 1;
        int T_before = T_after - 1;

        if (T_before == 0) {
            for (int v : parser.nets[e].cells) {
                if (v == movedCell || parser.cells[v].locked) continue;
                if (parser.cells[v].partition == fromP) {
                    if (!touched[v]) { touched[v] = 1; affected.push_back(v); }
                    delta[v] += 1;
                }
            }
        }
        else if (T_before == 1) {
            for (int v : parser.nets[e].cells) {
                if (v == movedCell || parser.cells[v].locked) continue;
                if (parser.cells[v].partition == toP) {
                    if (!touched[v]) { touched[v] = 1; affected.push_back(v); }
                    delta[v] -= 1;
                    break;
                }
            }
        }

        if (F_after == 0) {
            for (int v : parser.nets[e].cells) {
                if (v == movedCell || parser.cells[v].locked) continue;
                if (parser.cells[v].partition == toP) {
                    if (!touched[v]) { touched[v] = 1; affected.push_back(v); }
                    delta[v] -= 1;
                }
            }
        }
        else if (F_after == 1) {
            for (int v : parser.nets[e].cells) {
                if (v == movedCell || parser.cells[v].locked) continue;
                if (parser.cells[v].partition == fromP) {
                    if (!touched[v]) { touched[v] = 1; affected.push_back(v); }
                    delta[v] += 1;
                    break;
                }
            }
        }
    }

    for (int v : affected) {
        if (delta[v] != 0) {
            if (inBucket[v]) removeFromBucket(v);
            cellGain[v] += delta[v];
            addToBucket(v, cellGain[v]);
            delta[v] = 0;
        }
        touched[v] = 0;
    }
}

pair<int, int> FMPartitioner::findBestMove() {
    int maxGainIdx = (int)buckets.size() - 1;
    while (maxGainIdx >= 0) {
        for (int cellIdx = buckets[maxGainIdx]; cellIdx != -1; cellIdx = bucketNodes[cellIdx].next) {
            if (parser.cells[cellIdx].locked) continue;
            if (canMove(cellIdx)) {
                return {cellIdx, maxGainIdx - offset};
            }
        }
        maxGainIdx--;
    }
    return {-1, -1};
}

void FMPartitioner::makeMove(int cellIdx) {
    int fromP = parser.cells[cellIdx].partition;
    int toP = 1 - fromP;
    parser.cells[cellIdx].partition = toP;
    parser.cells[cellIdx].locked = true;

    int sz = parser.cells[cellIdx].size;
    partitionSizes[fromP] -= sz;
    partitionSizes[toP] += sz;

    for (int ni : cell2nets[cellIdx]) {
        netCount[ni][fromP]--;
        netCount[ni][toP]++;
    }
}

int FMPartitioner::fmPass() {
    initBuckets();

    vector<tuple<int,int,int>> moves;
    vector<int> sums;
    int sum = 0;
    int n = (int)parser.cells.size();

    for (int i = 0; i < n; i++) {
        auto [cellIdx, gain] = findBestMove();
        if (cellIdx == -1 && gain == -1) break;

        int fromP = parser.cells[cellIdx].partition;
        int toP = 1 - fromP;

        moves.push_back({cellIdx, fromP, toP});
        sum += gain;
        sums.push_back(sum);

        makeMove(cellIdx);
        removeFromBucket(cellIdx);
        updateGains(cellIdx);
    }

    if (moves.empty()) return 0;

    int maxGain = 0, best = 0;
    for (int i = 0; i < (int)sums.size(); i++) {
        if (sums[i] > maxGain) {
            maxGain = sums[i];
            best = i + 1;
        }
    }

    for (int i = (int)moves.size() - 1; i >= best; --i) {
        auto [c, f, t] = moves[i];
        parser.cells[c].partition = f;

        int sz = parser.cells[c].size;
        partitionSizes[f] += sz;
        partitionSizes[t] -= sz;

        for (int ni : cell2nets[c]) {
            netCount[ni][t]--;
            netCount[ni][f]++;
        }
    }

    for (auto& cell : parser.cells) cell.locked = false;
    return maxGain;
}

vector<int> FMPartitioner::greedyInit2Way(const Parser& P) {
    int n = (int)P.cells.size();
    vector<pair<int,int>> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i) v.push_back({P.cells[i].size, i});
    sort(v.rbegin(), v.rend());
    int sum[2] = {0, 0};
    vector<int> init(n, 0);
    for (auto [sz, idx] : v) {
        int g = (sum[0] <= sum[1]) ? 0 : 1;
        init[idx] = g;
        sum[g] += sz;
    }
    return init;
}

void FMPartitioner::buildSubParser(const vector<int>& subset, Parser& sub, vector<int>& new2old) {
    unordered_map<int,int> old2new;
    old2new.reserve(subset.size() * 2);

    new2old = subset;
    for (int i = 0; i < (int)subset.size(); ++i) old2new[ subset[i] ] = i;

    sub.cells.resize(subset.size());
    sub.totalSize = 0;
    sub.cellNameToIdx.clear();
    sub.nets.clear();
    sub.offset = 0;
    sub.cutSize = 0;

    for (int i = 0; i < (int)subset.size(); ++i) {
        int old = subset[i];
        sub.cells[i].name = parser.cells[old].name;
        sub.cells[i].size = parser.cells[old].size;
        sub.cells[i].id = i;
        sub.cells[i].partition = 0;
        sub.cells[i].locked = false;
        sub.cellNameToIdx[sub.cells[i].name] = i;
        sub.totalSize += sub.cells[i].size;
    }

    vector<int> countOffset(sub.cells.size(), 0);
    int thisGroup = parser.cells[subset.front()].partition;

    for (const auto& net : parser.nets) {
        if(netCount[net.id][1 - thisGroup] != 0) continue;

        Net nt;
        nt.name = net.name;
        nt.id = (int)sub.nets.size();
        for(auto cellIdx : net.cells){
            nt.cells.push_back(old2new[cellIdx]);
            countOffset[old2new[cellIdx]]++;
        }
        sub.nets.push_back(nt);
    }

    sub.offset = *max_element(countOffset.begin(), countOffset.end());
}

void FMPartitioner::mergeBack(int whichSide, const Parser& sub, const vector<int>& new2old) {
    for (int newIdx = 0; newIdx < (int)new2old.size(); ++newIdx) {
        int oldIdx = new2old[newIdx];
        int subPart = sub.cells[newIdx].partition;
        parser.cells[oldIdx].partition = (whichSide ? 2 : 0) + subPart;
    }
}

void FMPartitioner::partition(const vector<int>& init, int k, bool isParent) {
    K = k;
    int n = (int)parser.cells.size();

    fill(partitionSizes.begin(), partitionSizes.end(), 0);
    for(int i = 0; i < n; i++){
        auto &cell = parser.cells[i];
        cell.partition = init[i];
        partitionSizes[cell.partition] += cell.size;
    }

    parser.cutSize = computeCutSize();

    int maxZeroCounts = 3;
    for(int zeroCounts = 0, i = 0; zeroCounts < maxZeroCounts; i++){
        auto deltaVal = fmPass();
        if(deltaVal <= 0) zeroCounts++;
        parser.cutSize -= deltaVal;
    }

    if(k == 2) return;

    vector<int> idxA, idxB;
    idxA.reserve(parser.cells.size());
    idxB.reserve(parser.cells.size());
    for (int i = 0; i < (int)parser.cells.size(); ++i) {
        if (parser.cells[i].partition == 0) idxA.push_back(i);
        else idxB.push_back(i);
    }

    Parser subA, subB;
    vector<int> new2oldA, new2oldB;
    buildSubParser(idxA, subA, new2oldA);
    buildSubParser(idxB, subB, new2oldB);

    {
        FMPartitioner subFM_A(subA, lr, rr);
        auto initA = greedyInit2Way(subA);
        subFM_A.partition(initA, 2, false);
        mergeBack(0, subA, new2oldA);
    }
    {
        FMPartitioner subFM_B(subB, lr, rr);
        auto initB = greedyInit2Way(subB);
        subFM_B.partition(initB, 2, false);
        mergeBack(1, subB, new2oldB);
    }

    parser.cutSize = computeCutSize();
}

vector<int> makeGreedyRandomInit(const Parser& parser) {
    mt19937 rng(random_device{}());
    int n = (int)parser.cells.size();
    vector<pair<double, int>> cells;
    cells.reserve(n);

    uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < n; i++) {
        double perturbation_factor = 0.2;
        double randomized_size = parser.cells[i].size * (1.0 + dist(rng) * perturbation_factor);
        cells.push_back({randomized_size, i});
    }

    sort(cells.begin(), cells.end(), greater<pair<double, int>>());

    int sum[2] = {0, 0};
    vector<int> part(n);
    for (auto [_, idx] : cells) {
        int sz = parser.cells[idx].size;
        int g = (sum[0] <= sum[1]) ? 0 : 1;
        part[idx] = g;
        sum[g] += sz;
    }
    return part;
}
