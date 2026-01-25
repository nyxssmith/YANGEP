#!/bin/bash

# Source common variables
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common.sh"

scp $STEAM_DECK_USER@$STEAM_DECK_IP:/home/$STEAM_DECK_USER/YANGEP/input_log.txt ./input_log.txt
