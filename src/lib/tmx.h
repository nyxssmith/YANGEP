#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <pugixml.hpp>
#include <cute.h>
#include "tsx.h"

// Forward declarations
struct TMXTileset;
struct TMXLayer;
class Camera;

class tmx : public pugi::xml_document
{
private:
    std::string path;

    // Map properties
    int map_width;
    int map_height;
    int tile_width;
    int tile_height;

    // Tilesets used by this map
    std::vector<std::shared_ptr<TMXTileset>> tilesets;

    // Layers in this map
    std::vector<std::shared_ptr<TMXLayer>> layers;

    // Helper functions
    bool loadTilesets();
    bool loadLayers();
    std::shared_ptr<TMXTileset> findTilesetForGID(int gid) const;
    void parseCSVData(const std::string &csv_data, std::vector<int> &tile_data) const;

public:
    tmx() = default;
    tmx(const std::string &path);

    // Parse TMX file from given path
    bool parse(const std::string &path);

    // Debug function to print all TMX content
    void debugPrint() const;

    // Get map dimensions
    int getMapWidth() const { return map_width; }
    int getMapHeight() const { return map_height; }
    int getTileWidth() const { return tile_width; }
    int getTileHeight() const { return tile_height; }

    // Layer access
    int getLayerCount() const { return static_cast<int>(layers.size()); }
    std::shared_ptr<TMXLayer> getLayer(int index) const;
    std::shared_ptr<TMXLayer> getLayer(const std::string &name) const;

    // Tileset access
    int getTilesetCount() const { return static_cast<int>(tilesets.size()); }
    std::shared_ptr<TMXTileset> getTileset(int index) const;

    // Get a tile sprite at specific layer and map coordinates
    // Coordinate system: (0,0) is top-left, +X goes right, +Y goes down
    CF_Sprite getTileAt(int layer_index, int map_x, int map_y) const;
    CF_Sprite getTileAt(const std::string &layer_name, int map_x, int map_y) const;

    // Convert map coordinates to world coordinates
    // map_x, map_y: tile coordinates in the map (0,0 = top-left tile)
    // world_x, world_y: base world position offset
    // Returns: actual world position for the specified tile
    void mapToWorldCoords(int map_x, int map_y, float world_x, float world_y, float &tile_world_x, float &tile_world_y) const;

    // Convert world coordinates to map coordinates
    // Returns true if the world coordinates correspond to a valid map tile
    bool worldToMapCoords(float world_x, float world_y, float base_world_x, float base_world_y, int &map_x, int &map_y) const;

    // Render entire layer at given position
    void renderLayer(int layer_index, float world_x, float world_y) const;
    void renderLayer(const std::string &layer_name, float world_x, float world_y) const;

    // Render entire layer with camera-aware culling and positioning
    void renderLayer(int layer_index, const class Camera &camera, float world_x = 0.0f, float world_y = 0.0f) const;
    void renderLayer(const std::string &layer_name, const class Camera &camera, float world_x = 0.0f, float world_y = 0.0f) const;

    // Render all layers at given position
    void renderAllLayers(float world_x, float world_y) const;

    // Render all layers with camera-aware culling and positioning
    void renderAllLayers(const class Camera &camera, float world_x = 0.0f, float world_y = 0.0f) const;

    // Cache management
    void clearAllSpriteCaches();
};

// Structure to represent a tileset reference in the TMX
struct TMXTileset
{
    int first_gid;                 // First global tile ID for this tileset
    std::string source;            // Path to .tsx file (if external)
    std::string name;              // Tileset name
    std::shared_ptr<tsx> tsx_data; // The actual tileset data

    // Tile sprite cache to avoid regenerating the same sprites
    mutable std::map<int, CF_Sprite> sprite_cache;

    TMXTileset() : first_gid(0) {}

    // Convert global ID to local tile coordinates
    bool getLocalTileCoords(int gid, int &tile_x, int &tile_y) const;

    // Check if this tileset contains the given global ID
    bool containsGID(int gid) const;

    // Get sprite for given global ID (with caching)
    CF_Sprite getSpriteForGID(int gid) const;

    // Clear the sprite cache
    void clearCache();
};

// Structure to represent a layer in the TMX
// Coordinate system: (0,0) is top-left tile, +X goes right, +Y goes down
struct TMXLayer
{
    int id;                // Layer ID
    std::string name;      // Layer name
    int width;             // Layer width in tiles
    int height;            // Layer height in tiles
    bool visible;          // Layer visibility
    float opacity;         // Layer opacity (0.0 - 1.0)
    std::vector<int> data; // Tile data (global IDs) in row-major order

    TMXLayer() : id(0), width(0), height(0), visible(true), opacity(1.0f) {}

    // Get tile global ID at specific layer coordinates
    // x: horizontal position (0 = leftmost)
    // y: vertical position (0 = topmost)
    int getTileGID(int x, int y) const;

    // Check if coordinates are within layer bounds
    bool isValidCoordinate(int x, int y) const;
};
