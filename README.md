# Partition Solver (FM)

Build and run commands (run inside `src/`):
- `make` — builds `../bin/main`.
- `make run1`/`run2`/`run3`/`run4` — rebuilds, runs the four provided testcases (public1 2-way/4-way, public2 2-way/4-way), prints real elapsed time, and verifies output.
- Outputs are written to `../output/` and verified with `../verifier/verify`.
- `make clean` — removes built binaries in `../bin/`.

Manual execution:
```
./main <input file> <output file> <number of partitions>
```
Example from `bin/`:
```
./main ../testcase/public1.txt ../output/public1.2way.out 2
```
