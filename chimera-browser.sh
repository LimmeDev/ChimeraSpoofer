#!/bin/bash

# This script launches a web browser with a minimal font configuration
# to reduce the font fingerprinting surface.

export FONTCONFIG_FILE=$(pwd)/fonts.conf

# Default browser, can be changed to chrome, etc.
BROWSER=firefox

echo "--- Chimera Browser ---"
echo "Starting $BROWSER with minimal fonts..."
echo "-----------------------"

$BROWSER "$@" 