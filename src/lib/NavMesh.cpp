#include "NavMesh.h"
#include "tmx.h"
#include "CFNativeCamera.h"
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <queue>
#include <unordered_map>
#include <chrono>

NavMesh::NavMesh()
    : tile_width(32), tile_height(32), next_path_id(1)
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

bool NavMesh::addPoint(const std::string &name, CF_V2 position)
{
    // Check if point with this name already exists
    for (const auto &point : points)
    {
        if (point.name == name)
        {
            printf("NavMesh::addPoint - Point '%s' already exists\n", name.c_str());
            return false;
        }
    }

    // Find which polygon contains this position
    int poly_idx = findPolygonAt(position);

    if (poly_idx == -1)
    {
        printf("NavMesh::addPoint - Warning: Point '%s' at (%.1f, %.1f) is not on the navigation mesh\n",
               name.c_str(), position.x, position.y);
        // Still add it, but mark as not on mesh
    }

    points.push_back(NavMeshPoint(name, position, poly_idx));
    printf("NavMesh::addPoint - Added point '%s' at (%.1f, %.1f) [polygon: %d]\n",
           name.c_str(), position.x, position.y, poly_idx);
    return true;
}

bool NavMesh::removePoint(const std::string &name)
{
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        if (it->name == name)
        {
            points.erase(it);
            printf("NavMesh::removePoint - Removed point '%s'\n", name.c_str());
            return true;
        }
    }

    printf("NavMesh::removePoint - Point '%s' not found\n", name.c_str());
    return false;
}

const NavMeshPoint *NavMesh::getPoint(const std::string &name) const
{
    for (const auto &point : points)
    {
        if (point.name == name)
        {
            return &point;
        }
    }
    return nullptr;
}

void NavMesh::clearPoints()
{
    printf("NavMesh::clearPoints - Clearing %d points\n", static_cast<int>(points.size()));
    points.clear();
}

// Generate a path from start position to end position
std::shared_ptr<NavMeshPath> NavMesh::generatePath(CF_V2 start, CF_V2 end)
{
    auto path = std::make_shared<NavMeshPath>();

    // Snap start and end positions to valid tile centers
    int start_poly = findPolygonAt(start);
    int end_poly = findPolygonAt(end);

    if (start_poly == -1)
    {
        printf("NavMesh::generatePath - Start position (%.1f, %.1f) is not on navmesh\n", start.x, start.y);
        return path;
    }

    if (end_poly == -1)
    {
        printf("NavMesh::generatePath - End position (%.1f, %.1f) is not on navmesh\n", end.x, end.y);
        return path;
    }

    // Use the polygon centers as the actual start and end positions
    CF_V2 snapped_start = polygons[start_poly].center;
    CF_V2 snapped_end = polygons[end_poly].center;

    // Generate the path using snapped positions
    if (findPath(*path, snapped_start, snapped_end))
    {
        // Assign ID and add to tracked paths
        path->id = next_path_id++;
        paths.push_back(path);
        printf("NavMesh::generatePath - Path generated successfully (id: %d, total paths: %d)\n", path->id, static_cast<int>(paths.size()));
    }

    return path;
}

// Generate a path from start position to a named point
std::shared_ptr<NavMeshPath> NavMesh::generatePathToPoint(CF_V2 start, const std::string &point_name)
{
    // Get the named point from the navmesh
    const NavMeshPoint *point = getPoint(point_name);

    if (!point)
    {
        printf("NavMesh::generatePathToPoint - Point '%s' not found on navmesh\n", point_name.c_str());
        return std::make_shared<NavMeshPath>();
    }

    printf("NavMesh::generatePathToPoint - Pathfinding to point '%s' at (%.1f, %.1f)\n",
           point_name.c_str(), point->position.x, point->position.y);

    return generatePath(start, point->position);
}

// Helper function for pathfinding (A* implementation)
bool NavMesh::findPath(NavMeshPath &path, CF_V2 start, CF_V2 end) const
{
    auto start_time = std::chrono::high_resolution_clock::now();

    // Clear the path
    path.clear();

    int start_poly = findPolygonAt(start);
    int end_poly = findPolygonAt(end);

    if (start_poly == -1 || end_poly == -1)
    {
        return false;
    }

    // If start and end are in the same polygon, direct path
    if (start_poly == end_poly)
    {
        path.waypoints.push_back(start);
        path.waypoints.push_back(end);
        path.is_valid = true;
        path.calculateLength();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        printf("NavMesh::findPath - Direct path (same polygon), length: %.1f, time: %.3f ms\n",
               path.total_length, duration.count() / 1000.0);
        return true;
    }

    // Priority queue for A* (cost, polygon_index)
    struct Node
    {
        int poly_index;
        float g_cost; // Cost from start
        float h_cost; // Heuristic cost to end
        int parent;   // Parent polygon index

        float f_cost() const { return g_cost + h_cost; }

        bool operator>(const Node &other) const
        {
            return f_cost() > other.f_cost();
        }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
    std::unordered_map<int, Node> all_nodes;

    // Helper function to calculate heuristic (Euclidean distance)
    auto heuristic = [&](int poly_index) -> float
    {
        const NavPoly &poly = getPolygon(poly_index);
        CF_V2 diff = cf_v2(poly.center.x - end.x, poly.center.y - end.y);
        return cf_len(diff);
    };

    // Initialize start node
    Node start_node;
    start_node.poly_index = start_poly;
    start_node.g_cost = 0.0f;
    start_node.h_cost = heuristic(start_poly);
    start_node.parent = -1;

    open_set.push(start_node);
    all_nodes[start_poly] = start_node;

    // A* search
    while (!open_set.empty())
    {
        Node current = open_set.top();
        open_set.pop();

        // Found the goal
        if (current.poly_index == end_poly)
        {
            // Reconstruct path
            std::vector<int> poly_path;
            int current_poly = end_poly;

            while (current_poly != -1)
            {
                poly_path.push_back(current_poly);
                current_poly = all_nodes[current_poly].parent;
            }

            std::reverse(poly_path.begin(), poly_path.end());

            // Convert polygon path to waypoints
            path.waypoints.push_back(start);

            // Add polygon centers as waypoints
            for (size_t i = 1; i < poly_path.size(); i++)
            {
                const NavPoly &poly = getPolygon(poly_path[i]);
                path.waypoints.push_back(poly.center);
            }

            path.waypoints.push_back(end);
            path.is_valid = true;
            path.calculateLength();

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

            printf("NavMesh::findPath - Path found with %d waypoints, length: %.1f, time: %.3f ms\n",
                   path.getWaypointCount(), path.total_length, duration.count() / 1000.0);

            return true;
        }

        // Check all neighbors
        const NavPoly &current_poly = getPolygon(current.poly_index);

        for (int neighbor_index : current_poly.neighbors)
        {
            if (neighbor_index == -1)
                continue;

            const NavPoly &neighbor_poly = getPolygon(neighbor_index);

            // Calculate cost to move to this neighbor
            CF_V2 diff = cf_v2(neighbor_poly.center.x - current_poly.center.x,
                               neighbor_poly.center.y - current_poly.center.y);
            float move_cost = cf_len(diff);
            float new_g_cost = current.g_cost + move_cost;

            // Check if we found a better path to this neighbor
            auto it = all_nodes.find(neighbor_index);
            if (it == all_nodes.end() || new_g_cost < it->second.g_cost)
            {
                Node neighbor_node;
                neighbor_node.poly_index = neighbor_index;
                neighbor_node.g_cost = new_g_cost;
                neighbor_node.h_cost = heuristic(neighbor_index);
                neighbor_node.parent = current.poly_index;

                all_nodes[neighbor_index] = neighbor_node;
                open_set.push(neighbor_node);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    printf("NavMesh::findPath - No path found, time: %.3f ms\n", duration.count() / 1000.0);

    // No path found
    return false;
}

// Remove a path by its ID (marks as complete rather than removing)
bool NavMesh::removePathById(int path_id)
{
    for (auto &path : paths)
    {
        if (path && path->getId() == path_id)
        {
            path->markComplete();
            printf("NavMesh::removePathById - Marked path with id %d as complete\n", path_id);
            return true;
        }
    }

    printf("NavMesh::removePathById - Path with id %d not found\n", path_id);
    return false;
}

// Clear all tracked paths
void NavMesh::clearPaths()
{
    printf("NavMesh::clearPaths - Clearing %d paths\n", static_cast<int>(paths.size()));
    paths.clear();
}

void NavMesh::debugRenderPoints(const class CFNativeCamera &camera, CF_Color color) const
{
    if (points.empty())
        return;

    cf_draw_push_color(color);

    for (const auto &point : points)
    {
        // Create a small AABB around the point for visibility check
        const float point_radius = 5.0f;
        CF_Aabb point_bounds = make_aabb(
            cf_v2(point.position.x - point_radius, point.position.y - point_radius),
            cf_v2(point.position.x + point_radius, point.position.y + point_radius));

        // Only render if visible
        if (!camera.isVisible(point_bounds))
            continue;

        // Draw point as a circle (using a filled quad approximation)
        const float size = 8.0f; // Size of the point marker
        CF_Aabb point_rect = make_aabb(
            cf_v2(point.position.x - size / 2, point.position.y - size / 2),
            cf_v2(point.position.x + size / 2, point.position.y + size / 2));

        cf_draw_quad_fill(point_rect, 0.0f);

        // Draw a border around the point for better visibility
        cf_draw_push_color(cf_make_color_rgb(0, 0, 0)); // Black border
        cf_draw_quad(point_rect, 0.0f, 1.5f);
        cf_draw_pop_color();

        // Optionally draw the point name above it
        // Note: You may want to adjust text rendering based on your setup
        // draw_text(point.name.c_str(), cf_v2(point.position.x, point.position.y + size));
    }

    cf_draw_pop_color();
}
