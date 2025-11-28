#!/usr/bin/env bash
set -euo pipefail

# Quick installer for profiling dependencies on a fresh Ubuntu machine.
# Installs: build tools, gperftools (libprofiler), graphviz (for SVG),
# and Go + the latest pprof (Go version).

echo "[info] Updating apt index..."
sudo apt-get update

echo "[info] Installing packages: build-essential gperftools graphviz Go..."
sudo apt-get install -y build-essential google-perftools libgoogle-perftools-dev graphviz golang-go pkg-config

echo "[info] Installing Go pprof (auto-picks a version your Go can build)..."
export GO111MODULE=on

PPROF_CANDIDATES=(
  "github.com/google/pprof@latest"                    # may require newer Go
  "github.com/google/pprof@v0.0.0-20230822-2c05c54e7e5c"
  "github.com/google/pprof@v0.0.0-20211214055906-6f57359322fd"  # conservative fallback
)

PPROF_OK=false
for mod in "${PPROF_CANDIDATES[@]}"; do
  echo "[info] trying: go install $mod"
  if go install "$mod"; then
    PPROF_OK=true
    echo "[done] installed pprof via $mod"
    break
  else
    echo "[warn] failed to install $mod (will try next candidate)" >&2
  fi
done

if [[ "$PPROF_OK" != true ]]; then
  echo "[error] failed to install pprof with available candidates. Consider upgrading Go or installing manually." >&2
fi

if ! grep -q 'export PATH="$HOME/go/bin:$PATH"' "$HOME/.bashrc" 2>/dev/null; then
  echo 'export PATH="$HOME/go/bin:$PATH"' >> "$HOME/.bashrc"
  echo "[info] Added GOPATH bin to ~/.bashrc (run: source ~/.bashrc)"
else
  echo "[info] GOPATH bin already in ~/.bashrc"
fi

echo "[done] Dependencies installed. You may need to open a new shell or run: source ~/.bashrc"
