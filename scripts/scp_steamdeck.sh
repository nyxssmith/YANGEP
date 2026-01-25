#!/bin/bash

# Source common variables
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/common.sh"

# Create YANGEP directory on Steam Deck if it doesn't exist
ssh $STEAM_DECK_USER@$STEAM_DECK_IP "mkdir -p YANGEP"

scp -r $BUILD_DIR/assets $STEAM_DECK_USER@$STEAM_DECK_IP:/home/$STEAM_DECK_USER/YANGEP/assets
scp -r $BUILD_DIR/$EXEC_NAME $STEAM_DECK_USER@$STEAM_DECK_IP:/home/$STEAM_DECK_USER/YANGEP/$EXEC_NAME
# use steamdeck specific window config
scp -r $BUILD_DIR/assets/window-config.steamdeck.json $STEAM_DECK_USER@$STEAM_DECK_IP:/home/$STEAM_DECK_USER/YANGEP/assets/window-config.json

# mark executable
ssh $STEAM_DECK_USER@$STEAM_DECK_IP "chmod +x /home/$STEAM_DECK_USER/YANGEP/$EXEC_NAME"