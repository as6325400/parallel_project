#include <iostream>
#include <utility>
#include <vector>

#include "parser.h"
#include "fm_partitioner.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input> <output> <k>\n";
        return 1;
    }

    Parser parser;
    parser.readInput(argv[1]);

    int k = atoi(argv[3]);
    auto [lb, rb] = make_pair(0.45, 0.55);
    if (k == 4) { lb = 0.48; rb = 0.52; }

    auto init = makeGreedyRandomInit(parser);

    FMPartitioner fm(parser, lb, rb);
    fm.partition(init, k, true);

    parser.writeOutput(argv[2], k);
    return 0;
}
