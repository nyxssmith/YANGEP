#include "NavMeshPath.h"
#include "NavMesh.h"
#include "NavMeshPoint.h"
#include "CFNativeCamera.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <cfloat>
#include <chrono>

NavMeshPath::NavMeshPath()
    : id(0), is_valid(false), total_length(0.0f), currentWaypointIndex(0), has_debug_color(false)
{
}

NavMeshPath::~NavMeshPath()
{
    clear();
}

void NavMeshPath::clear()
{
    waypoints.clear();
    is_valid = false;
    total_length = 0.0f;
    currentWaypointIndex = 0;
}

bool NavMeshPath::generate(const NavMesh &navmesh, CF_V2 start, CF_V2 end)
{
    auto start_time = std::chrono::high_resolution_clock::now();

    clear();

    // Check if start and end are on the navmesh
    int start_poly = navmesh.findPolygonAt(start);
    int end_poly = navmesh.findPolygonAt(end);

    if (start_poly == -1)
    {
        printf("NavMeshPath::generate - Start position (%.1f, %.1f) is not on navmesh\n", start.x, start.y);
        return false;
    }

    if (end_poly == -1)
    {
        printf("NavMeshPath::generate - End position (%.1f, %.1f) is not on navmesh\n", end.x, end.y);
        return false;
    }

    // If start and end are in the same polygon, direct path
    if (start_poly == end_poly)
    {
        waypoints.push_back(start);
        waypoints.push_back(end);
        is_valid = true;
        calculateLength();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        printf("NavMeshPath::generate - Direct path (same polygon), length: %.1f, time: %.3f ms\n",
               total_length, duration.count() / 1000.0);
        return true;
    }

    // Use A* pathfinding to find path through polygons
    is_valid = findPath(navmesh, start, end);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    if (is_valid)
    {
        calculateLength();
        printf("NavMeshPath::generate - Path found with %d waypoints, length: %.1f, time: %.3f ms\n",
               getWaypointCount(), total_length, duration.count() / 1000.0);
    }
    else
    {
        printf("NavMeshPath::generate - No path found, time: %.3f ms\n", duration.count() / 1000.0);
    }

    return is_valid;
}

bool NavMeshPath::generateToPoint(const NavMesh &navmesh, CF_V2 start, const std::string &point_name)
{
    // Get the named point from the navmesh
    const NavMeshPoint *point = navmesh.getPoint(point_name);

    if (!point)
    {
        printf("NavMeshPath::generateToPoint - Point '%s' not found on navmesh\n", point_name.c_str());
        clear();
        return false;
    }

    printf("NavMeshPath::generateToPoint - Pathfinding to point '%s' at (%.1f, %.1f)\n",
           point_name.c_str(), point->position.x, point->position.y);

    return generate(navmesh, start, point->position);
}

bool NavMeshPath::findPath(const NavMesh &navmesh, CF_V2 start, CF_V2 end)
{
    // A* pathfinding implementation through navigation mesh polygons

    int start_poly = navmesh.findPolygonAt(start);
    int end_poly = navmesh.findPolygonAt(end);

    if (start_poly == -1 || end_poly == -1)
    {
        return false;
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
        const NavPoly &poly = navmesh.getPolygon(poly_index);
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
            waypoints.push_back(start);

            // Add polygon centers as waypoints
            for (size_t i = 1; i < poly_path.size(); i++)
            {
                const NavPoly &poly = navmesh.getPolygon(poly_path[i]);
                waypoints.push_back(poly.center);
            }

            waypoints.push_back(end);

            return true;
        }

        // Check all neighbors
        const NavPoly &current_poly = navmesh.getPolygon(current.poly_index);

        for (int neighbor_index : current_poly.neighbors)
        {
            if (neighbor_index == -1)
                continue;

            const NavPoly &neighbor_poly = navmesh.getPolygon(neighbor_index);

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

    // No path found
    return false;
}

void NavMeshPath::calculateLength()
{
    total_length = 0.0f;

    for (size_t i = 1; i < waypoints.size(); i++)
    {
        CF_V2 diff = cf_v2(waypoints[i].x - waypoints[i - 1].x,
                           waypoints[i].y - waypoints[i - 1].y);
        total_length += cf_len(diff);
    }
}

CF_V2 NavMeshPath::getWaypoint(int index) const
{
    if (index < 0 || index >= static_cast<int>(waypoints.size()))
    {
        return cf_v2(0, 0);
    }
    return waypoints[index];
}

// Get the current waypoint without advancing
CF_V2 *NavMeshPath::getCurrent()
{
    // Check if we have a valid current waypoint
    if (currentWaypointIndex < 0 || currentWaypointIndex + 2 >= static_cast<int>(waypoints.size()))
    {
        return nullptr; // No current waypoint
    }

    // Return pointer to the current waypoint
    return &waypoints[currentWaypointIndex];
}

// Get the next waypoint and advance to it
CF_V2 *NavMeshPath::getNext()
{
    // Check if there is a next waypoint
    // printf("NavMeshPath::getNext - comparing currentWaypointIndex (%d) + 2 (%d) >= waypoints.size() (%zu)\n",
    //       currentWaypointIndex, currentWaypointIndex + 2, waypoints.size());
    if (currentWaypointIndex + 2 >= static_cast<int>(waypoints.size()))
    {
        return nullptr; // No next waypoint
    }

    // Advance to the next waypoint
    currentWaypointIndex++;

    // if currentWaypointIndex is equal to size, return nullptr
    if (currentWaypointIndex >= static_cast<int>(waypoints.size()))
    {
        return nullptr;
    }

    // Return pointer to the new current waypoint
    return &waypoints[currentWaypointIndex];
}

// Check if a location is at the current waypoint
bool NavMeshPath::isAtCurrentWaypoint(CF_V2 location, float tolerance) const
{
    // Check if we have a valid path and current waypoint
    if (!is_valid || currentWaypointIndex < 0 || currentWaypointIndex >= static_cast<int>(waypoints.size()))
    {
        return false;
    }

    // Get the current waypoint
    CF_V2 currentWaypoint = waypoints[currentWaypointIndex];

    // Calculate distance to current waypoint
    CF_V2 diff = cf_v2(location.x - currentWaypoint.x, location.y - currentWaypoint.y);
    float distance = cf_len(diff);

    // Check if within tolerance
    return distance <= tolerance;
}

void NavMeshPath::debugRender(const CFNativeCamera &camera) const
{
    if (!is_valid || waypoints.size() < 2)
        return;

    // Assign a random color on first render
    if (!has_debug_color)
    {
        debug_color = cf_make_color_rgb(
            static_cast<uint8_t>(50 + (rand() % 206)), // 50-255 to avoid too dark colors
            static_cast<uint8_t>(50 + (rand() % 206)),
            static_cast<uint8_t>(50 + (rand() % 206)));
        has_debug_color = true;
    }

    cf_draw_push_color(debug_color);

    // Draw lines connecting waypoints
    for (size_t i = 1; i < waypoints.size(); i++)
    {
        CF_V2 start = waypoints[i - 1];
        CF_V2 end = waypoints[i];

        // Create AABB for visibility check
        float min_x = std::min(start.x, end.x);
        float min_y = std::min(start.y, end.y);
        float max_x = std::max(start.x, end.x);
        float max_y = std::max(start.y, end.y);

        CF_Aabb line_bounds = make_aabb(cf_v2(min_x - 1, min_y - 1), cf_v2(max_x + 1, max_y + 1));

        // Only render if visible
        if (!camera.isVisible(line_bounds))
            continue;

        // Draw path segment as thick line
        cf_draw_line(start, end, 3.0f);
    }

    // Draw waypoint markers (small circles)
    for (const auto &waypoint : waypoints)
    {
        const float marker_size = 6.0f;
        CF_Aabb marker_bounds = make_aabb(
            cf_v2(waypoint.x - marker_size, waypoint.y - marker_size),
            cf_v2(waypoint.x + marker_size, waypoint.y + marker_size));

        if (!camera.isVisible(marker_bounds))
            continue;

        // Draw as filled circle (approximated with quad)
        cf_draw_quad_fill(marker_bounds, 0.0f);
    }

    cf_draw_pop_color();
}
