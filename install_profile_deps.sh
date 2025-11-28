#!/usr/bin/env bash
set -euo pipefail

# Quick installer for profiling dependencies on a fresh Ubuntu machine.
# Installs: build tools, gperftools (libprofiler), graphviz (for SVG),
# and Go + the latest pprof (Go version).

echo "[info] Updating apt index..."
sudo apt-get update

echo "[info] Installing packages: build-essential gperftools graphviz Go..."
sudo apt-get install -y build-essential google-perftools libgoogle-perftools-dev graphviz golang-go pkg-config

echo "[info] Installing Go pprof..."
export GO111MODULE=on
go install github.com/google/pprof@latest

if ! grep -q 'export PATH="$HOME/go/bin:$PATH"' "$HOME/.bashrc" 2>/dev/null; then
  echo 'export PATH="$HOME/go/bin:$PATH"' >> "$HOME/.bashrc"
  echo "[info] Added GOPATH bin to ~/.bashrc (run: source ~/.bashrc)"
else
  echo "[info] GOPATH bin already in ~/.bashrc"
fi

echo "[done] Dependencies installed. You may need to open a new shell or run: source ~/.bashrc"
