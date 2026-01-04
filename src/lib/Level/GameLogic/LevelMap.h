#pragma once

#include "../FileHandling/tmx.h"
#include <vector>
#include <memory>

// Forward declarations
class CFNativeCamera;
class DataFile;

/**
 * StructureLayer - Extended TMX layer for game structures
 *
 * Similar to TMXLayer but includes additional game logic fields
 * like the lowest world Y coordinate for vertical positioning.
 */
struct StructureLayer
{
    int id;                     // Layer ID
    std::string name;           // Layer name
    int width;                  // Layer width in tiles
    int height;                 // Layer height in tiles
    bool visible;               // Layer visibility
    float opacity;              // Layer opacity (0.0 - 1.0)
    std::vector<int> data;      // Tile data (global IDs) in row-major order
    int lowestWorldYCoordinate; // Lowest world Y coordinate for this structure layer

private:
    std::shared_ptr<TMXLayer> tmxLayer; // Original TMX layer for rendering

public:
    StructureLayer() : id(0), width(0), height(0), visible(true), opacity(1.0f), lowestWorldYCoordinate(0) {}

    // Create StructureLayer from TMXLayer
    explicit StructureLayer(const TMXLayer &tmxLayer);

    // Get the underlying TMX layer for rendering
    std::shared_ptr<TMXLayer> getTMXLayer() const { return tmxLayer; }

    // Get tile global ID at specific layer coordinates
    // x: horizontal position (0 = leftmost)
    // y: vertical position (0 = topmost)
    int getTileGID(int x, int y) const;

    // Check if coordinates are within layer bounds
    bool isValidCoordinate(int x, int y) const;
};

/**
 * LevelMap - Extended TMX map class for game logic
 *
 * This class extends the tmx class to add game-specific functionality
 * while maintaining all TMX file handling capabilities.
 * For now, it's a simple wrapper but will be extended with additional features.
 */
class LevelMap : public tmx
{
private:
    // Structure layers for multi-level rendering
    std::vector<std::shared_ptr<StructureLayer>> structures;

    // Override to filter structure layers from regular layers
    bool loadLayers() override;

public:
    /**
     * Default constructor
     */
    LevelMap() = default;

    /**
     * Constructor - loads TMX file from path
     * @param path Path to the TMX file
     */
    explicit LevelMap(const std::string &path);

    /**
     * Destructor
     */
    ~LevelMap() = default;

    /**
     * Get the number of structure layers
     * @return Number of structure layers
     */
    int getStructureCount() const { return static_cast<int>(structures.size()); }

    /**
     * Get a structure layer by index
     * @param index Index of the structure layer
     * @return Shared pointer to the structure layer, or nullptr if out of bounds
     */
    std::shared_ptr<StructureLayer> getStructure(int index) const;

    /**
     * Add a structure layer
     * @param structure The structure layer to add
     */
    void addStructure(std::shared_ptr<StructureLayer> structure);

    /**
     * Render a single TMX layer
     * @param layer The TMX layer to render
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     * @param worldX World X offset
     * @param worldY World Y offset
     */
    void renderSingleLayer(std::shared_ptr<TMXLayer> layer, const CFNativeCamera &camera, const DataFile &config, float worldX = 0.0f, float worldY = 0.0f) const;
};
