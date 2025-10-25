#pragma once

#include <vector>
#include <string>
#include <cute.h>

// Forward declarations
class NavMesh;
class CFNativeCamera;

// Structure to represent a path through the navigation mesh
class NavMeshPath
{
private:
    std::vector<CF_V2> waypoints; // Path waypoints (world positions)
    bool is_valid;                // Whether the path is valid and reachable
    float total_length;           // Total length of the path

    // Internal pathfinding function
    bool findPath(const NavMesh &navmesh, CF_V2 start, CF_V2 end);

    // Helper function to calculate path length
    void calculateLength();

public:
    NavMeshPath();
    ~NavMeshPath();

    // Generate a path from start position to end position using the navmesh
    bool generate(const NavMesh &navmesh, CF_V2 start, CF_V2 end);

    // Generate a path from start position to a named point on the navmesh
    bool generateToPoint(const NavMesh &navmesh, CF_V2 start, const std::string &point_name);

    // Clear the path
    void clear();

    // Query functions
    bool isValid() const { return is_valid; }
    int getWaypointCount() const { return static_cast<int>(waypoints.size()); }
    float getLength() const { return total_length; }
    const std::vector<CF_V2> &getWaypoints() const { return waypoints; }

    // Get waypoint at specific index
    CF_V2 getWaypoint(int index) const;

    // Debug rendering
    void debugRender(const CFNativeCamera &camera, CF_Color color = cf_make_color_rgb(255, 255, 0)) const;
};
