#!/bin/bash
source "$(dirname "$0")/common.sh"
ln -sf "assets" "$BUILD_DIR/assets"
cd "$BUILD_DIR"
./$EXEC_NAME