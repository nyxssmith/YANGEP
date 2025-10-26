#!/bin/bash
source "$(dirname "$0")/common.sh"
cd "$BUILD_DIR"
# if assets exists, remove it
if [[ -d "assets" ]]; then
    rm -rf assets
fi