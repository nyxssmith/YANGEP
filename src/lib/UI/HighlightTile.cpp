#include "HighlightTile.h"
#include "LevelV1.h"

using namespace Cute;

void highlightArea(CF_Aabb bounds, CF_Color color,
                   float border_opacity, float fill_opacity)
{
    // Create the inner bounds (1 pixel inset for the fill area)
    constexpr float border_thickness = 1.0f;
    CF_Aabb inner_bounds = make_aabb(
        cf_v2(bounds.min.x + border_thickness, bounds.min.y + border_thickness),
        cf_v2(bounds.max.x - border_thickness, bounds.max.y - border_thickness));

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
    cf_draw_quad(bounds, border_thickness, 0.0f);
    cf_draw_pop_color();
}

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

    // Create the tile bounds
    CF_Aabb tile_bounds = make_aabb(
        cf_v2(tile_center_x - half_width, tile_center_y - half_height),
        cf_v2(tile_center_x + half_width, tile_center_y + half_height));

    // Use highlightArea to do the actual rendering
    highlightArea(tile_bounds, color, border_opacity, fill_opacity);
}

void highlightAreaHalves(CF_Aabb bounds, CF_Color left_color, CF_Color right_color,
                         float border_opacity, float fill_opacity)
{
    // Calculate the middle x coordinate
    float mid_x = (bounds.min.x + bounds.max.x) / 2.0f;

    // Create bounds for left and right halves
    CF_Aabb left_bounds = make_aabb(
        cf_v2(bounds.min.x, bounds.min.y),
        cf_v2(mid_x, bounds.max.y));

    CF_Aabb right_bounds = make_aabb(
        cf_v2(mid_x, bounds.min.y),
        cf_v2(bounds.max.x, bounds.max.y));

    constexpr float border_thickness = 1.0f;

    // Draw left half fill
    CF_Aabb left_inner = make_aabb(
        cf_v2(left_bounds.min.x + border_thickness, left_bounds.min.y + border_thickness),
        cf_v2(left_bounds.max.x, left_bounds.max.y - border_thickness));

    CF_Color left_fill = left_color;
    left_fill.a = fill_opacity;
    cf_draw_push_color(left_fill);
    cf_draw_quad_fill(left_inner, 0.0f);
    cf_draw_pop_color();

    // Draw right half fill
    CF_Aabb right_inner = make_aabb(
        cf_v2(right_bounds.min.x, right_bounds.min.y + border_thickness),
        cf_v2(right_bounds.max.x - border_thickness, right_bounds.max.y - border_thickness));

    CF_Color right_fill = right_color;
    right_fill.a = fill_opacity;
    cf_draw_push_color(right_fill);
    cf_draw_quad_fill(right_inner, 0.0f);
    cf_draw_pop_color();

    // Draw border in segments - left side with left color, right side with right color
    CF_Color left_border = left_color;
    left_border.a = border_opacity;

    CF_Color right_border = right_color;
    right_border.a = border_opacity;

    // Left edge
    cf_draw_push_color(left_border);
    cf_draw_line(cf_v2(bounds.min.x, bounds.min.y), cf_v2(bounds.min.x, bounds.max.y), border_thickness);
    // Top-left half
    cf_draw_line(cf_v2(bounds.min.x, bounds.max.y), cf_v2(mid_x, bounds.max.y), border_thickness);
    // Bottom-left half
    cf_draw_line(cf_v2(bounds.min.x, bounds.min.y), cf_v2(mid_x, bounds.min.y), border_thickness);
    cf_draw_pop_color();

    // Right edge
    cf_draw_push_color(right_border);
    cf_draw_line(cf_v2(bounds.max.x, bounds.min.y), cf_v2(bounds.max.x, bounds.max.y), border_thickness);
    // Top-right half
    cf_draw_line(cf_v2(mid_x, bounds.max.y), cf_v2(bounds.max.x, bounds.max.y), border_thickness);
    // Bottom-right half
    cf_draw_line(cf_v2(mid_x, bounds.min.y), cf_v2(bounds.max.x, bounds.min.y), border_thickness);
    cf_draw_pop_color();
}

void highlightTileHalves(const LevelV1 &level, int tile_x, int tile_y,
                         CF_Color left_color, CF_Color right_color,
                         float border_opacity, float fill_opacity)
{
    // Get tile and map dimensions from the level
    float tile_width = static_cast<float>(level.getTileWidth());
    float tile_height = static_cast<float>(level.getTileHeight());

    // Convert tile coordinates to world coordinates
    float tile_center_x = tile_x * tile_width;
    float tile_center_y = tile_y * tile_height;

    float half_width = tile_width / 2.0f;
    float half_height = tile_height / 2.0f;

    // Create the tile bounds
    CF_Aabb tile_bounds = make_aabb(
        cf_v2(tile_center_x - half_width, tile_center_y - half_height),
        cf_v2(tile_center_x + half_width, tile_center_y + half_height));

    // Use highlightAreaHalves to do the actual rendering
    highlightAreaHalves(tile_bounds, left_color, right_color, border_opacity, fill_opacity);
}
