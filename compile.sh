#!/bin/bash

# Rewritten to use the Mint build tool.
# Requires: pip install mint-build (>=0.2.0)

set -e

echo "--- Building Chimera with Mint ---"

# Release build; pass through any extra args supplied to this wrapper.
mint build -r "$@"

BIN_PATH=$(mint out-path 2>/dev/null || true)
if [[ -n "$BIN_PATH" && -f "$BIN_PATH" ]]; then
  echo "--- Build complete: $BIN_PATH ---"
else
  echo "--- Build complete. Binary is inside the build directory ---"
fi

echo "Run with: sudo ./build/*/chimera (or whatever output path was configured)" 