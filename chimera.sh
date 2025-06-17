#!/bin/bash
# Wrapper script to run commands with the Chimera interceptor

# Get the directory where the script is located
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
INTERCEPTOR_PATH="$SCRIPT_DIR/interceptor.so"

if [ ! -f "$INTERCEPTOR_PATH" ]; then
    echo "Error: Interceptor library not found at $INTERCEPTOR_PATH"
    echo "Please run 'make interceptor' first."
    exit 1
fi

if [ $# -eq 0 ]; then
    echo "Usage: ./chimera.sh <program> [args...]"
    exit 1
fi

echo "--- CHIMERA INTERCEPTOR ---"
echo "Preloading: $INTERCEPTOR_PATH"
echo "Running command: $@"
echo "---------------------------"

export LD_PRELOAD="$INTERCEPTOR_PATH"
exec "$@" 