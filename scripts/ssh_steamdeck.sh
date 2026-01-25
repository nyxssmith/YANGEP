#!/bin/bash

# Source common variables
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common.sh"

ssh $STEAM_DECK_USER@$STEAM_DECK_IP
