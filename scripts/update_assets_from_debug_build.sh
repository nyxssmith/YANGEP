#!/bin/bash
source "$(dirname "$0")/common.sh"
# copy all assets from 
if [[ -d "$BUILD_DIR/assets" ]]; then
    rm -rf assets
    cp -r "$BUILD_DIR/assets" .
else
    echo "No assets directory found in debug build."
    exit 1
fi