# Partition Solver (FM)

## Build & Run
(from `src/`)
- `make` — builds `../bin/main`.
- `make run1` / `run2` / `run3` / `run4` — rebuilds, runs the four testcases (public1 2-way/4-way, public2 2-way/4-way), prints real elapsed time, and verifies output via `../verifier/verify`.
- Outputs are written to `../output/`.
- `make clean` — removes built binaries in `../bin/`.

Manual execution (from `bin/`):
```
./main <input file> <output file> <number of partitions>
```
Example:
```
./main ../testcase/public1.txt ../output/public1.2way.out 2
```

## Profiling (gperftools + pprof)
### One-step dependency install (Ubuntu)
From the project root:
```
bash install_profile_deps.sh
source ~/.bashrc    # or open a new shell to refresh PATH
```
Installs: build tools, gperftools/libprofiler, graphviz, Go, and the latest Go `pprof` (in `$HOME/go/bin`).

### Generate a CPU profile
From the project root (auto-builds `bin/main_profiled` with libprofiler):
```
bash profile.sh [input] [output] [k] [profile_out] [http_port]
```
Defaults: `input=./testcase/public1.txt`, `output=./output/profile.out`, `k=2`, `profile_out=./output/cpu.pprof`, `http_port` empty (no web UI).

Example (public2, k=2, start web on 8080):
```
bash profile.sh ./testcase/public2.txt ./output/profile.out 2 ./output/cpu.pprof 8080
```
The script will:
- Build `bin/main_profiled` (linked with libprofiler).
- Run and write `./output/cpu.pprof`.
- If `pprof` is available: print a text top summary; if `dot` is available, also write `./output/profile.svg`; if `http_port` is set, launch the pprof web UI (open `http://localhost:<port>`).

Manual inspection (if you already have `cpu.pprof`):
```
pprof --text ./bin/main_profiled ./output/cpu.pprof | head
pprof -http=:8080 ./bin/main_profiled ./output/cpu.pprof
```

Clean profiling artifacts:
```
./profile.sh clean   # removes bin/main_profiled, output/profile.out, output/cpu.pprof, output/profile.svg
```
