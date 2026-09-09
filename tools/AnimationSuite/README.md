# Crab Maker

## Basic Usage

Run `python3 devtools-launcher.py` to open a launcher listing each tool below with a short description and a button to launch it.

Used to create a crab animation from a set of source images for the crab body parts. The source images are mapped to define where the attachment points are for each body part, and then a crab config is used to define which mapped images to use for each body part.

Crab configs define the style of each limb of a crab, for example a red crab with blue claws. Crab configs are stored in `crab_configs/` and are json files that map the body part names to the style config names.

Animations are created with `make_animation.py`, which will prompt for an animation name and can be used to stop-motion the crab in a certain direction, or to create a walking animation in a certain direction. Each animation can be for a single direction.
Animations are stored as `.json` files in `animation_configs/` and can be used by `output_from_animation.py` to create a single image with all frames of the animation in a row, given an input `--crab-config`. Frames are output to `output_imgs` in a row.

## Folder Overview

#### Source Images

Contains the source images for the crab body parts.

`map_images.py` will prompt to map every source image in this folder and save the mapping to `mapped_imgs/`.

#### Mapped Images

Contains copies of the source images, paired with a json for where its attachment point(s) are. The json is made by `map_images.py`.

##### Style Configs

Each one of these is used to define a style or set of images, for example, a blue crab or a red crab. Each style config is a json that maps the body part names to the mapped image names.

```json
{
  "body_facing_down": "<name of .json file in mapped_imgs/ for body facing down for that style>",
  "body_facing_up": "<name of .json file in mapped_imgs/ for body facing up for that style>",
  "body_facing_right": "<name of .json file in mapped_imgs/ for body facing right for that style>",
  "claw_facing_up": "<name of .json file in mapped_imgs/ for claw facing up for that style>",
  "claw_facing_down": "<name of .json file in mapped_imgs/ for claw facing down for that style>",
  "claw_facing_right": "<name of .json file in mapped_imgs/ for claw facing right for that style>",
  "leg_facing_up": "<name of .json file in mapped_imgs/ for leg facing up for that style>",
  "leg_facing_right": "<name of .json file in mapped_imgs/ for leg facing right for that style>"
}
```

#### Crab Configs

Contains json made by `make_crab_config.py` that maps the body part names to the style config names.

#### Animation Configs

See [basic usage](#basic-usage) for more information. Each animation config is a json that defines the frames of the animation, and the direction of the animation.
