#include "HighlightTile.h"
#include "LevelV1.h"

using namespace Cute;

void highlightTile(const LevelV1 &level, int tile_x, int tile_y, CF_Color color,
                   float border_opacity, float fill_opacity)
{
    // Get tile and map dimensions from the level
    float tile_width = static_cast<float>(level.getTileWidth());
    float tile_height = static_cast<float>(level.getTileHeight());

    // Convert tile coordinates to world coordinates
    // tile_x: 0 = left column, increases rightward
    // tile_y: 0 = bottom row, increases upward (engine convention)
    //
    // NavMesh uses tile CENTER as (tile_x * tile_width, tile_y * tile_height)
    // with vertices at center ± half_width/half_height
    // So tile (0,0) center is at world (0, 0), with bounds from (-half, -half) to (+half, +half)
    float tile_center_x = tile_x * tile_width;
    float tile_center_y = tile_y * tile_height;

    float half_width = tile_width / 2.0f;
    float half_height = tile_height / 2.0f;

    // Create the outer tile bounds (full tile)
    CF_Aabb outer_bounds = make_aabb(
        cf_v2(tile_center_x - half_width, tile_center_y - half_height),
        cf_v2(tile_center_x + half_width, tile_center_y + half_height));

    // Create the inner bounds (1 pixel inset for the fill area)
    constexpr float border_thickness = 1.0f;
    CF_Aabb inner_bounds = make_aabb(
        cf_v2(tile_center_x - half_width + border_thickness, tile_center_y - half_height + border_thickness),
        cf_v2(tile_center_x + half_width - border_thickness, tile_center_y + half_height - border_thickness));

    // Draw the interior fill with lower opacity
    CF_Color fill_color = color;
    fill_color.a = fill_opacity;
    cf_draw_push_color(fill_color);
    cf_draw_quad_fill(inner_bounds, 0.0f);
    cf_draw_pop_color();

    // Draw the border using a quad outline with higher opacity
    CF_Color border_color = color;
    border_color.a = border_opacity;
    cf_draw_push_color(border_color);
    cf_draw_quad(outer_bounds, border_thickness, 0.0f);
    cf_draw_pop_color();
}
