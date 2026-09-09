#!/bin/bash
source "$(dirname "$0")/common.sh"

current_dir="$(pwd)"

cd "$ANIMATION_TOOLS_DIR"

# check if pillow is in current virtual environment
if ! python -c "import PIL" &> /dev/null; then
    echo "Pillow is not installed in the virtual environment!"
    if [[ -d "venv" ]]; then
        source venv/bin/activate
    else
        echo "Virtual environment not found!"
        exit 1
    fi
fi
set -e
# for each base animation name
base_animation_names=("idle")

for base_animation_name in "${base_animation_names[@]}"; do
    echo "Rendering base animation: $base_animation_name"
    # get all crab configs from the crab_configs directory
    crab_configs=(crab_configs/*.json)
    for crab_config in "${crab_configs[@]}"; do
        echo "Using crab config: $crab_config"
        crab_config_name="$(basename "$crab_config" .json)"
        # render each direction
        python3 output_from_animation.py --size 128 --crab-config "$(basename "$crab_config")" --animation "$base_animation_name"_up --cli-only
        python3 output_from_animation.py --size 128 --crab-config "$(basename "$crab_config")" --animation "$base_animation_name"_down --cli-only
        python3 output_from_animation.py --size 128 --crab-config "$(basename "$crab_config")" --animation "$base_animation_name"_left --cli-only
        python3 output_from_animation.py --size 128 --crab-config "$(basename "$crab_config")" --animation "$base_animation_name"_right --cli-only
        # combine
        python3 combine_animation_slices.py --up "output_imgs/${base_animation_name}_${crab_config_name}_up.png" \
                            --down "output_imgs/${base_animation_name}_${crab_config_name}_down.png" \
                            --left "output_imgs/${base_animation_name}_${crab_config_name}_left.png" \
                            --right "output_imgs/${base_animation_name}_${crab_config_name}_right.png" \
                            --output "${base_animation_name}/${crab_config_name}.png"

    done
done

set -ex

# go back to project root
cd "$current_dir"
# for all animations, copy them to the assets directory
for base_animation_name in "${base_animation_names[@]}"; do
    echo "Copying $base_animation_name to assets"
    cp "$ANIMATION_TOOLS_DIR/output_imgs/$base_animation_name"/*.png "assets/Art/AnimationsSheets/$base_animation_name"
done