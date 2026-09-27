# Make A Level

Make a folder per level
`details.json` should be placed inside each level folder.

```json
{
  "name": "spaceship"
}
```

Create a sub folder called `tiles` and put all tilemap pngs used by the level inside it.

Next, create a new tmx project in Tiled for the level. **It should have tile 0,0 as the top left corner, and increase in values to the right and down.** (default behavior in Tiled)

In Tiled, create a new tileset using the images from the `tiles` folder. and save it as a .tsx file in the level folder.

Next you can draw the level.

Special layers:

- Any layer called `navmesh` (case insensitive) will be used as the navigation mesh for AI pathfinding and where the player can walk
- Any layer with `structure` in its name will be treated like a billboard sprite, similar to the player and other entities (used for objects that will occlude the player and other entities)
- Any layer with:
  - `cutb` for cut bottom
  - `cutt` for cut top
  - `cutr` for cut right
  - `cutl` for cut left
  - Will cut the navmesh on that edge of all tiles placed in it.

To put entities in the level, create an `entities.json` file inside the level folder with the following structure:

````json
{
"entities": [
{
"name": "test_enemy_crab",
"position": { "x": 10, "y": 10 },
"path": "/assets/DataFiles/Entities/test_enemy_crab"
}
]
}
```
to put a new entity in the level at position 10,10
````
