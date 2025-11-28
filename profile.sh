#!/usr/bin/env bash
set -euo pipefail

# Build and run with gperftools CPU profiler.
# Usage:
#   ./profile.sh [input] [output] [k] [profile_out] [http_port]
# Defaults:
#   input=./testcase/public1.txt
#   output=./output/profile.out
#   k=2
#   profile_out=./output/cpu.pprof
#   http_port="" (if set, e.g., 8080, will launch pprof web UI)

ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$ROOT/src"
BIN_DIR="$ROOT/bin"
OUT_DIR="$ROOT/output"

INPUT="${1:-"$ROOT/testcase/public1.txt"}"
OUTPUT="${2:-"$OUT_DIR/profile.out"}"
K="${3:-2}"
PROFILE_OUT="${4:-"$OUT_DIR/cpu.pprof"}"
HTTP_PORT="${5:-""}"

if [[ "$INPUT" == "clean" ]]; then
  echo "[clean] removing profiled binary and profile outputs"
  rm -f "$BIN_DIR/main_profiled" "$OUT_DIR/profile.out" "$OUT_DIR/cpu.pprof" "$OUT_DIR/profile.svg"
  exit 0
fi

mkdir -p "$BIN_DIR" "$OUT_DIR"

CXX="${CXX:-g++}"
CXXFLAGS="-std=c++17 -O2 -g -fno-omit-frame-pointer -fopenmp"
LDFLAGS="-Wl,--no-as-needed -lprofiler -Wl,--as-needed"
PROFILE_BIN="$BIN_DIR/main_profiled"

echo "[build] $PROFILE_BIN"
"$CXX" $CXXFLAGS "$SRC_DIR/main.cpp" "$SRC_DIR/parser.cpp" "$SRC_DIR/fm_partitioner.cpp" -o "$PROFILE_BIN" $LDFLAGS

echo "[run] input=$INPUT output=$OUTPUT k=$K"
echo "[run] profile -> $PROFILE_OUT"
CPUPROFILE="$PROFILE_OUT" "$PROFILE_BIN" "$INPUT" "$OUTPUT" "$K"

if command -v pprof >/dev/null 2>&1; then
  echo "[pprof] top 20:"
  pprof --text "$PROFILE_BIN" "$PROFILE_OUT" | head -n 50
  if command -v dot >/dev/null 2>&1; then
    if pprof --svg "$PROFILE_BIN" "$PROFILE_OUT" > "$OUT_DIR/profile.svg"; then
      echo "[pprof] flame graph -> $OUT_DIR/profile.svg"
    fi
  fi
  if [[ -n "$HTTP_PORT" ]]; then
    echo "[pprof] starting web UI at http://localhost:${HTTP_PORT}"
    pprof -http=":${HTTP_PORT}" "$PROFILE_BIN" "$PROFILE_OUT"
  fi
else
  echo "[warn] pprof not found in PATH. Install gperftools pprof to inspect $PROFILE_OUT" >&2
fi
