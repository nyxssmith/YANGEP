#!/bin/bash
source "$(dirname "$0")/common.sh"


# for each crab config
for crab_config in $ANIMATION_TOOLS_DIR/crab_configs/*.json; do
    echo "Processing crab config: $crab_config"
    crab_config_name="$(basename "$crab_config" .json)"
    # convert crab config to entity

    # ensure the folder exists
    mkdir -p assets/DataFiles/Entities/$crab_config_name

    # if "character.json" exists in that folder, skip
    if [ -f "assets/DataFiles/Entities/$crab_config_name/character.json" ]; then
        echo "Character already exists for $crab_config_name, skipping..."
        continue
    fi

    # echo this multiline json to the character.json file
    echo "{
  \"name\": \"$crab_config_name\",
  \"hitbox_size\": 32,
  \"hitbox_distance\": 0,
  \"innate_actions\": [],
  \"layers\": [
    {
      \"filename\": \"$crab_config_name.png\",
      \"tile_size\": 128
    }
  ]
}" > "assets/DataFiles/Entities/$crab_config_name/character.json"
done
