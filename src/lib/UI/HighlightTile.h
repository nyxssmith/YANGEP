#pragma once

#include <cute.h>

// Forward declaration
class LevelV1;

/**
 * Highlights an arbitrary rectangular area with a border and fill.
 *
 * @param bounds The axis-aligned bounding box to highlight
 * @param color The base color to highlight the area with
 * @param border_opacity The opacity of the 1-pixel border (0.0 to 1.0, default 0.9)
 * @param fill_opacity The opacity of the interior fill (0.0 to 1.0, default 0.4)
 */
void highlightArea(CF_Aabb bounds, CF_Color color,
                   float border_opacity = 0.9f, float fill_opacity = 0.4f);

/**
 * Highlights an arbitrary rectangular area split down the middle with two colors.
 *
 * @param bounds The axis-aligned bounding box to highlight
 * @param left_color The color for the left half
 * @param right_color The color for the right half
 * @param border_opacity The opacity of the 1-pixel border (0.0 to 1.0, default 0.9)
 * @param fill_opacity The opacity of the interior fill (0.0 to 1.0, default 0.4)
 */
void highlightAreaHalves(CF_Aabb bounds, CF_Color left_color, CF_Color right_color,
                         float border_opacity = 0.9f, float fill_opacity = 0.4f);

/**
 * Highlights a single tile on the tilemap with a border and fill.
 *
 * @param level The level to use for tile dimensions and map size
 * @param tile_x The x coordinate of the tile (0 = left column)
 * @param tile_y The y coordinate of the tile (0 = bottom row)
 * @param color The base color to highlight the tile with
 * @param border_opacity The opacity of the 1-pixel border (0.0 to 1.0, default 0.9)
 * @param fill_opacity The opacity of the interior fill (0.0 to 1.0, default 0.4)
 */
void highlightTile(const LevelV1 &level, int tile_x, int tile_y, CF_Color color,
                   float border_opacity = 0.9f, float fill_opacity = 0.4f);

/**
 * Highlights a single tile on the tilemap split down the middle with two colors.
 *
 * @param level The level to use for tile dimensions and map size
 * @param tile_x The x coordinate of the tile (0 = left column)
 * @param tile_y The y coordinate of the tile (0 = bottom row)
 * @param left_color The color for the left half
 * @param right_color The color for the right half
 * @param border_opacity The opacity of the 1-pixel border (0.0 to 1.0, default 0.9)
 * @param fill_opacity The opacity of the interior fill (0.0 to 1.0, default 0.4)
 */
void highlightTileHalves(const LevelV1 &level, int tile_x, int tile_y,
                         CF_Color left_color, CF_Color right_color,
                         float border_opacity = 0.9f, float fill_opacity = 0.4f);
