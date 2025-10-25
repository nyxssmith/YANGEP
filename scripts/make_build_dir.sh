#!/bin/bash

source "$(dirname "$0")/common.sh"
if [ -d "$BUILD_DIR" ]; then
    exit 0
else
    mkdir -p "$BUILD_DIR"
fi
# initialize the build directory
if [[ "$OSTYPE" == "darwin"* ]]; then
    cd "$BUILD_DIR" && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
elif [[ "$OSTYPE" == "linux"* ]]; then
    cd "$BUILD_DIR" && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-include cstdint" ..
fi