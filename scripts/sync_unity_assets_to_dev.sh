#!/bin/bash

set -euo pipefail

usage() {
	echo "Usage: $0 [--to-nyx | --to-aria]"
}

target=""

for arg in "$@"; do
	case "$arg" in
		--to-nyx)
			target="nyx"
			;;
		--to-aria)
			target="aria"
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			echo "Unknown option: $arg"
			usage
			exit 1
			;;
	esac
done

if [[ -z "$target" ]]; then
	echo "Missing target option."
	usage
	exit 1
fi

RSYNC_IP=""
RSYNC_USER=""
TARGET_DIR=""
ASSETS_SRC_DIR="assets/Art/AnimationsSheets/UnityAssetsRaw"

if [[ "$target" == "nyx" ]]; then
	echo "sending assets to nyx"
    RSYNC_IP="192.168.1.171"
    RSYNC_USER="nyx"
    TARGET_DIR="/home/nyx/Git/YANGEP/assets/Art/AnimationsSheets/UnityAssetsRaw"
elif [[ "$target" == "aria" ]]; then
	echo "sending assets to aria"
    RSYNC_IP="192.168.1.181"
    RSYNC_USER="aria"
    TARGET_DIR="/Users/aria/games/YANGEP/assets/Art/AnimationsSheets/UnityAssetsRaw"
fi

# Sync the assets using rsync
if [[ "$RSYNC_IP" == "todo" || "$TARGET_DIR" == "todo" ]]; then
	echo "Target '$target' is not fully configured yet."
	exit 1
fi

rsync -avz --delete -e ssh "$ASSETS_SRC_DIR/" "$RSYNC_USER@$RSYNC_IP:$TARGET_DIR/"
