#pragma once

#include <string>
#include <cute.h>

// Structure to represent a named point on the navigation mesh
// Useful for spawn points, waypoints, objectives, etc.
struct NavMeshPoint
{
    std::string name;  // Identifier for this point (e.g., "spawn_player", "waypoint_1")
    CF_V2 position;    // World position of the point
    int polygon_index; // Index of polygon containing this point (-1 if not on mesh)

    NavMeshPoint();
    NavMeshPoint(const std::string &n, CF_V2 pos);
    NavMeshPoint(const std::string &n, CF_V2 pos, int poly_idx);
};
