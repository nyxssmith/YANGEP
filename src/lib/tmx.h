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
class DataFile;

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

    // Layer highlighting configuration (layer name -> should highlight)
    std::map<std::string, bool> layer_highlight_map;

    // Layer border highlighting configuration (layer name -> should highlight borders only)
    std::map<std::string, bool> layer_border_highlight_map;

    // Cached border edges for each layer (layer index -> vector of edge AABBs)
    mutable std::map<int, std::vector<CF_Aabb>> layer_border_cache;

    // Helper functions
    bool loadTilesets();
    bool loadLayers();
    std::shared_ptr<TMXTileset> findTilesetForGID(int gid) const;
    void parseCSVData(const std::string &csv_data, std::vector<int> &tile_data) const;

    // Calculate border edges for a layer (returns AABBs for each border tile)
    std::vector<CF_Aabb> calculateLayerBorderEdges(int layer_index, float world_x, float world_y) const;

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
    // Note: TMX uses (0,0) top-left, +Y down, but rendering converts to Y-up coordinate system
    CF_Sprite getTileAt(int layer_index, int map_x, int map_y) const;
    CF_Sprite getTileAt(const std::string &layer_name, int map_x, int map_y) const;

    // Convert map coordinates to world coordinates
    // map_x, map_y: tile coordinates in TMX format (0,0 = top-left tile)
    // world_x, world_y: base world position offset
    // Returns: actual world position for rendering (with Y-axis flipped from TMX)
    void mapToWorldCoords(int map_x, int map_y, float world_x, float world_y, float &tile_world_x, float &tile_world_y) const;

    // Convert world coordinates to map coordinates
    // Returns true if the world coordinates correspond to a valid map tile
    // Note: Handles conversion from rendering Y-up to TMX Y-down coordinate system
    bool worldToMapCoords(float world_x, float world_y, float base_world_x, float base_world_y, int &map_x, int &map_y) const;

    // Render entire layer at given position
    void renderLayer(int layer_index, float world_x, float world_y) const;
    void renderLayer(const std::string &layer_name, float world_x, float world_y) const;

    // Render entire layer with camera-aware culling and positioning
    void renderLayer(int layer_index, const class CFNativeCamera &camera, float world_x = 0.0f, float world_y = 0.0f) const;
    void renderLayer(const std::string &layer_name, const class CFNativeCamera &camera, float world_x = 0.0f, float world_y = 0.0f) const;

    // Render layer with highlighting option
    void renderLayer(int layer_index, const class CFNativeCamera &camera, bool highlight_tiles, float world_x = 0.0f, float world_y = 0.0f) const;

    // Render all layers at given position
    void renderAllLayers(float world_x, float world_y) const;

    // Render all layers with camera-aware culling and positioning
    void renderAllLayers(const class CFNativeCamera &camera, float world_x = 0.0f, float world_y = 0.0f) const;

    // Render all layers with camera and config for layer highlighting
    void renderAllLayers(const class CFNativeCamera &camera, const class DataFile &config, float world_x = 0.0f, float world_y = 0.0f) const;

    // Configure layer highlighting from config (processes once and stores in map)
    void setLayerHighlightConfig(const class DataFile &config);

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
// Note: TMX coordinate system (0,0 top-left, Y down) but rendering uses Y-up
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
