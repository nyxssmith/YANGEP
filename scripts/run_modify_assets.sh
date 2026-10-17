#!/bin/bash
source "$(dirname "$0")/common.sh"
START_DIR="$(pwd)"
cd "$BUILD_DIR"
./$EXEC_NAME
cd "$START_DIR"
read -p "Overwrite existing assets with changes from execution? (y/N) " OVERWRITE
if [[ "$OVERWRITE" =~ ^[Yy]$ ]]; then
	./scripts/update_assets_from_debug_build.sh
fi