#include "NavMesh.h"
#include "tmx.h"
#include "CFNativeCamera.h"
#include <algorithm>
#include <cmath>
#include <cfloat>

NavMesh::NavMesh()
    : tile_width(32), tile_height(32)
{
    bounds = make_aabb(cf_v2(0, 0), cf_v2(0, 0));
}

NavMesh::~NavMesh()
{
    clear();
}

void NavMesh::clear()
{
    polygons.clear();
    edges.clear();
    bounds = make_aabb(cf_v2(0, 0), cf_v2(0, 0));
}

bool NavMesh::buildFromLayer(const std::shared_ptr<TMXLayer> &layer,
                             int tile_width, int tile_height,
                             float world_x, float world_y,
                             bool invert)
{
    if (!layer)
    {
        printf("NavMesh::buildFromLayer - Invalid layer\n");
        return false;
    }

    clear();

    this->tile_width = tile_width;
    this->tile_height = tile_height;

    printf("NavMesh: Building from layer '%s' (%dx%d tiles)\n",
           layer->name.c_str(), layer->width, layer->height);

    // Create a boolean grid of walkable tiles
    std::vector<bool> walkable_tiles(layer->width * layer->height, false);

    for (int y = 0; y < layer->height; y++)
    {
        for (int x = 0; x < layer->width; x++)
        {
            int gid = layer->getTileGID(x, y);
            int index = y * layer->width + x;

            // Determine if this tile is walkable
            if (invert)
            {
                // Empty tiles (GID == 0) are walkable
                walkable_tiles[index] = (gid == 0);
            }
            else
            {
                // Filled tiles (GID != 0) are walkable
                walkable_tiles[index] = (gid != 0);
            }
        }
    }

    // Generate navigation mesh from the walkable tile grid
    generateFromTileGrid(walkable_tiles, layer->width, layer->height, world_x, world_y);

    printf("NavMesh: Generated %d polygons and %d edges\n",
           getPolygonCount(), getEdgeCount());

    return getPolygonCount() > 0;
}

bool NavMesh::buildFromLayer(const tmx &map, const std::string &layer_name,
                             float world_x, float world_y,
                             bool invert)
{
    // First try to get from navmesh layers
    auto layer = map.getNavMeshLayer(layer_name);

    // If not found in navmesh layers, try regular layers
    if (!layer)
    {
        layer = map.getLayer(layer_name);
    }

    if (!layer)
    {
        printf("NavMesh::buildFromLayer - Layer '%s' not found in navmesh or regular layers\n", layer_name.c_str());
        return false;
    }

    return buildFromLayer(layer, map.getTileWidth(), map.getTileHeight(), world_x, world_y, invert);
}

CF_V2 NavMesh::tileToWorld(int tile_x, int tile_y, float world_x, float world_y) const
{
    // Convert from TMX tile coordinates to world coordinates (centered on tile)
    // Note: This assumes the same coordinate conversion as TMX rendering
    float wx = world_x + (tile_x * tile_width) + (tile_width / 2.0f);
    float wy = world_y + (tile_y * tile_height) + (tile_height / 2.0f);
    return cf_v2(wx, wy);
}

void NavMesh::generateFromTileGrid(const std::vector<bool> &walkable_tiles,
                                   int grid_width, int grid_height,
                                   float world_x, float world_y)
{
    // Simple approach: Create one rectangle polygon per walkable tile
    // This can be optimized later by merging adjacent tiles

    float min_x = FLT_MAX, min_y = FLT_MAX;
    float max_x = -FLT_MAX, max_y = -FLT_MAX;

    for (int y = 0; y < grid_height; y++)
    {
        for (int x = 0; x < grid_width; x++)
        {
            int index = y * grid_width + x;

            if (!walkable_tiles[index])
                continue;

            // Create a rectangular polygon for this tile
            NavPoly poly;

            // Calculate world coordinates for tile corners
            // Convert from TMX coordinate system (Y down) to rendering system (Y up)
            float tile_world_x = world_x + (x * tile_width);
            float tile_world_y = world_y + ((grid_height - 1 - y) * tile_height);

            float half_width = tile_width / 2.0f;
            float half_height = tile_height / 2.0f;

            // Create vertices in counter-clockwise order (bottom-left, bottom-right, top-right, top-left)
            poly.vertices.push_back(cf_v2(tile_world_x - half_width, tile_world_y - half_height)); // Bottom-left
            poly.vertices.push_back(cf_v2(tile_world_x + half_width, tile_world_y - half_height)); // Bottom-right
            poly.vertices.push_back(cf_v2(tile_world_x + half_width, tile_world_y + half_height)); // Top-right
            poly.vertices.push_back(cf_v2(tile_world_x - half_width, tile_world_y + half_height)); // Top-left

            // Calculate center
            poly.center = cf_v2(tile_world_x, tile_world_y);

            // Update bounds
            min_x = std::min(min_x, tile_world_x - half_width);
            min_y = std::min(min_y, tile_world_y - half_height);
            max_x = std::max(max_x, tile_world_x + half_width);
            max_y = std::max(max_y, tile_world_y + half_height);

            polygons.push_back(poly);

            // Create edges for this tile
            for (size_t i = 0; i < poly.vertices.size(); i++)
            {
                size_t next_i = (i + 1) % poly.vertices.size();
                NavEdge edge(poly.vertices[i], poly.vertices[next_i], static_cast<int>(polygons.size() - 1), -1);
                edges.push_back(edge);
            }
        }
    }

    // Set bounds
    bounds = make_aabb(cf_v2(min_x, min_y), cf_v2(max_x, max_y));

    // Calculate neighbor relationships
    calculateNeighbors();

    printf("NavMesh: Bounds: (%.1f, %.1f) to (%.1f, %.1f)\n",
           bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y);
}

void NavMesh::calculateNeighbors()
{
    // Simple approach: Check for shared edges between polygons
    // Two polygons are neighbors if they share at least one edge

    const float EPSILON = 0.1f; // Tolerance for floating point comparison

    for (size_t i = 0; i < polygons.size(); i++)
    {
        polygons[i].neighbors.clear();

        for (size_t j = 0; j < polygons.size(); j++)
        {
            if (i == j)
                continue;

            // Check if polygons i and j share an edge
            bool shares_edge = false;

            for (size_t vi = 0; vi < polygons[i].vertices.size() && !shares_edge; vi++)
            {
                size_t vi_next = (vi + 1) % polygons[i].vertices.size();
                CF_V2 edge_i_start = polygons[i].vertices[vi];
                CF_V2 edge_i_end = polygons[i].vertices[vi_next];

                for (size_t vj = 0; vj < polygons[j].vertices.size() && !shares_edge; vj++)
                {
                    size_t vj_next = (vj + 1) % polygons[j].vertices.size();
                    CF_V2 edge_j_start = polygons[j].vertices[vj];
                    CF_V2 edge_j_end = polygons[j].vertices[vj_next];

                    // Check if edges are the same (potentially reversed)
                    CF_V2 diff1 = cf_v2(edge_i_start.x - edge_j_start.x, edge_i_start.y - edge_j_start.y);
                    CF_V2 diff2 = cf_v2(edge_i_end.x - edge_j_end.x, edge_i_end.y - edge_j_end.y);
                    CF_V2 diff3 = cf_v2(edge_i_start.x - edge_j_end.x, edge_i_start.y - edge_j_end.y);
                    CF_V2 diff4 = cf_v2(edge_i_end.x - edge_j_start.x, edge_i_end.y - edge_j_start.y);

                    bool same_edge = (cf_len(diff1) < EPSILON && cf_len(diff2) < EPSILON);
                    bool reversed_edge = (cf_len(diff3) < EPSILON && cf_len(diff4) < EPSILON);

                    if (same_edge || reversed_edge)
                    {
                        shares_edge = true;
                    }
                }
            }

            if (shares_edge)
            {
                polygons[i].neighbors.push_back(static_cast<int>(j));
            }
        }
    }

    // Debug: Print neighbor information for first few polygons
    int debug_count = std::min(5, static_cast<int>(polygons.size()));
    for (int i = 0; i < debug_count; i++)
    {
        printf("NavMesh: Polygon %d has %d neighbors\n",
               i, static_cast<int>(polygons[i].neighbors.size()));
    }
}

void NavMesh::calculateCentroids()
{
    for (auto &poly : polygons)
    {
        if (poly.vertices.empty())
            continue;

        float sum_x = 0.0f;
        float sum_y = 0.0f;

        for (const auto &vertex : poly.vertices)
        {
            sum_x += vertex.x;
            sum_y += vertex.y;
        }

        poly.center = cf_v2(sum_x / poly.vertices.size(), sum_y / poly.vertices.size());
    }
}

int NavMesh::findPolygonAt(CF_V2 point) const
{
    // Check if point is within bounds first
    if (!cf_contains_point(bounds, point))
    {
        return -1;
    }

    // Check each polygon to see if it contains the point
    for (size_t i = 0; i < polygons.size(); i++)
    {
        const NavPoly &poly = polygons[i];

        // Use ray casting algorithm to determine if point is inside polygon
        bool inside = false;
        size_t j = poly.vertices.size() - 1;

        for (size_t k = 0; k < poly.vertices.size(); k++)
        {
            const CF_V2 &vi = poly.vertices[k];
            const CF_V2 &vj = poly.vertices[j];

            if (((vi.y > point.y) != (vj.y > point.y)) &&
                (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x))
            {
                inside = !inside;
            }

            j = k;
        }

        if (inside)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool NavMesh::isWalkable(CF_V2 point) const
{
    return findPolygonAt(point) != -1;
}

void NavMesh::debugRender(const class CFNativeCamera &camera) const
{
    // Render both polygons and edges with default colors
    // Green semi-transparent for polygons, red for edges
    CF_Color poly_color = cf_make_color_rgba(0, 255, 0, 76);  // Green with ~30% opacity
    CF_Color edge_color = cf_make_color_rgba(255, 0, 0, 204); // Red with ~80% opacity
    debugRenderPolygons(camera, poly_color);
    debugRenderEdges(camera, edge_color);
}

void NavMesh::debugRenderPolygons(const class CFNativeCamera &camera, CF_Color color) const
{
    cf_draw_push_color(color);

    for (const auto &poly : polygons)
    {
        // Create AABB for visibility check
        float min_x = FLT_MAX, min_y = FLT_MAX;
        float max_x = -FLT_MAX, max_y = -FLT_MAX;

        for (const auto &vertex : poly.vertices)
        {
            min_x = std::min(min_x, vertex.x);
            min_y = std::min(min_y, vertex.y);
            max_x = std::max(max_x, vertex.x);
            max_y = std::max(max_y, vertex.y);
        }

        CF_Aabb poly_bounds = make_aabb(cf_v2(min_x, min_y), cf_v2(max_x, max_y));

        // Only render if visible
        if (!camera.isVisible(poly_bounds))
            continue;

        // Draw filled polygon (simplified as quad for rectangles)
        if (poly.vertices.size() == 4)
        {
            // Draw as filled quad
            cf_draw_quad_fill(poly_bounds, 0.0f);
        }
        else
        {
            // Draw as triangle fan for other polygons
            // (This is a simplified approach, proper triangulation would be better)
            for (size_t i = 1; i + 1 < poly.vertices.size(); i++)
            {
                cf_draw_tri_fill(poly.vertices[0], poly.vertices[i], poly.vertices[i + 1], 0.0f);
            }
        }
    }

    cf_draw_pop_color();
}

void NavMesh::debugRenderEdges(const class CFNativeCamera &camera, CF_Color color) const
{
    cf_draw_push_color(color);

    for (const auto &edge : edges)
    {
        // Create AABB for edge visibility check
        float min_x = std::min(edge.start.x, edge.end.x);
        float min_y = std::min(edge.start.y, edge.end.y);
        float max_x = std::max(edge.start.x, edge.end.x);
        float max_y = std::max(edge.start.y, edge.end.y);

        CF_Aabb edge_bounds = make_aabb(cf_v2(min_x - 1, min_y - 1), cf_v2(max_x + 1, max_y + 1));

        // Only render if visible
        if (!camera.isVisible(edge_bounds))
            continue;

        // Draw edge as a line
        cf_draw_line(edge.start, edge.end, 2.0f); // 2px thick line
    }

    cf_draw_pop_color();
}
