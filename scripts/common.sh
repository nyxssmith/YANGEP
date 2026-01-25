#!/bin/bash

# Define common variables
BUILD_DIR="build"
VERSION_FILE="scripts/version.txt"
VERSIONS_FILE="scripts/versions.txt"
VERSION_DIR="versions"
EXEC_NAME="yangep"
ACTION_EDITOR_EXEC_NAME="yangep_action_editor"
# this assumes that you have ssh keys set up for steamdeck in authorized_keys
STEAM_DECK_IP="192.168.1.163"
STEAM_DECK_USER="deck"