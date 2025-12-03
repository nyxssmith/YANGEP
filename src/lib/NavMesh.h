#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cute.h>
#include "NavMeshPoint.h"
#include "NavMeshPath.h"

// Forward declarations
struct TMXLayer;
class tmx;

// Structure to represent a navigation polygon (convex polygon)
struct NavPoly
{
    std::vector<CF_V2> vertices; // Vertices in counter-clockwise order
    CF_V2 center;                // Centroid of the polygon
    std::vector<int> neighbors;  // Indices of neighboring polygons

    NavPoly() : center(cf_v2(0, 0)) {}
};

// Structure to represent a navigation edge (for portals between polygons)
struct NavEdge
{
    CF_V2 start;
    CF_V2 end;
    int poly_a; // Index of polygon on one side (-1 if boundary)
    int poly_b; // Index of polygon on other side (-1 if boundary)

    NavEdge() : start(cf_v2(0, 0)), end(cf_v2(0, 0)), poly_a(-1), poly_b(-1) {}
    NavEdge(CF_V2 s, CF_V2 e, int a = -1, int b = -1)
        : start(s), end(e), poly_a(a), poly_b(b) {}
};

// Main NavMesh class for pathfinding and navigation
class NavMesh
{
private:
    std::vector<NavPoly> polygons;                   // Navigation polygons
    std::vector<NavEdge> edges;                      // All edges in the mesh
    std::vector<NavMeshPoint> points;                // Named points on the mesh
    std::vector<std::shared_ptr<NavMeshPath>> paths; // All paths generated on this mesh
    int next_path_id;                                // Next path ID to assign (starts at 1)
    CF_Aabb bounds;                                  // Bounding box of the entire mesh
    int tile_width;                                  // Tile width from TMX
    int tile_height;                                 // Tile height from TMX

    // Helper functions for mesh generation
    void generateFromTileGrid(const std::vector<bool> &walkable_tiles,
                              int grid_width, int grid_height,
                              float world_x, float world_y);
    void mergeAdjacentTiles();
    void triangulate();
    void calculateNeighbors();
    void calculateCentroids();

    // Helper function for pathfinding (A* implementation)
    bool findPath(NavMeshPath &path, CF_V2 start, CF_V2 end) const;

    // Convert tile grid coordinates to world coordinates
    CF_V2 tileToWorld(int tile_x, int tile_y, float world_x, float world_y) const;

public:
    NavMesh();
    ~NavMesh();

    // Construct navigation mesh from a TMX layer
    // Walkable tiles are those with GID != 0 (or optionally GID == 0 depending on use case)
    // world_x, world_y: base world position offset
    bool buildFromLayer(const std::shared_ptr<TMXLayer> &layer,
                        int tile_width, int tile_height,
                        float world_x = 0.0f, float world_y = 0.0f,
                        bool invert = false); // invert: true = empty tiles are walkable, false = filled tiles are walkable

    // Construct navigation mesh from a TMX layer by name
    bool buildFromLayer(const tmx &map, const std::string &layer_name,
                        float world_x = 0.0f, float world_y = 0.0f,
                        bool invert = false);

    // Clear the navigation mesh
    void clear();

    // Query functions
    int getPolygonCount() const { return static_cast<int>(polygons.size()); }
    int getEdgeCount() const { return static_cast<int>(edges.size()); }
    const NavPoly &getPolygon(int index) const { return polygons[index]; }
    const NavEdge &getEdge(int index) const { return edges[index]; }
    CF_Aabb getBounds() const { return bounds; }

    // Find which polygon contains a given world point
    // Returns -1 if point is not in any polygon
    int findPolygonAt(CF_V2 point) const;

    // Check if a world point is walkable
    bool isWalkable(CF_V2 point) const;

    // NavMesh point management
    // Add a point to the mesh (automatically finds containing polygon)
    bool addPoint(const std::string &name, CF_V2 position);

    // Remove a point by name
    bool removePoint(const std::string &name);

    // Get a point by name (returns nullptr if not found)
    const NavMeshPoint *getPoint(const std::string &name) const;

    // Get all points
    const std::vector<NavMeshPoint> &getPoints() const { return points; }
    int getPointCount() const { return static_cast<int>(points.size()); }

    // Clear all points
    void clearPoints();

    // Path generation and management
    // Generate a path from start position to end position
    // Returns a shared pointer to the path (may be invalid if no path found)
    std::shared_ptr<NavMeshPath> generatePath(CF_V2 start, CF_V2 end);

    // Generate a path from start position to a named point
    // Returns a shared pointer to the path (may be invalid if no path found)
    std::shared_ptr<NavMeshPath> generatePathToPoint(CF_V2 start, const std::string &point_name);

    // Get all paths generated on this mesh
    const std::vector<std::shared_ptr<NavMeshPath>> &getPaths() const { return paths; }
    int getPathCount() const { return static_cast<int>(paths.size()); }

    // Remove a path by its ID
    // Returns true if the path was found and removed, false otherwise
    bool removePathById(int path_id);

    // Clear all tracked paths
    void clearPaths();

    // Debug rendering
    void debugRender(const class CFNativeCamera &camera) const;
    void debugRenderPolygons(const class CFNativeCamera &camera, CF_Color color = cf_color_white()) const;
    void debugRenderEdges(const class CFNativeCamera &camera, CF_Color color = cf_color_red()) const;
    void debugRenderPoints(const class CFNativeCamera &camera, CF_Color color = cf_make_color_rgb(255, 255, 0)) const;
};
