#pragma once

#include <string>
#include <memory>
#include "tmx.h"
#include "NavMesh.h"
#include "DataFile.h"

// Forward declarations
class CFNativeCamera;

/**
 * LevelV1 - A level loader and manager class
 *
 * Loads and initializes all level components from a directory:
 * - TMX tile map file
 * - NavMesh from the TMX navmesh layer
 * - entities.json data file
 * - details.json data file
 */
class LevelV1
{
private:
    std::string levelDirectory;
    std::string levelName;

    // Core level components
    std::unique_ptr<tmx> levelMap;
    std::unique_ptr<NavMesh> navmesh;

    // Level data files
    DataFile entities;
    DataFile details;

    // TMX tile dimensions (cached for convenience)
    int tileWidth;
    int tileHeight;

    // Initialization status
    bool initialized;

public:
    /**
     * Constructor - initializes all level components from a directory
     * @param directoryPath Path to the level directory (e.g., "/assets/Levels/test_two")
     */
    explicit LevelV1(const std::string &directoryPath);

    /**
     * Destructor
     */
    ~LevelV1() = default;

    /**
     * Check if the level was successfully initialized
     * @return true if all components loaded successfully, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * Get the level name (extracted from directory or details.json)
     * @return Level name string
     */
    const std::string &getLevelName() const { return levelName; }

    /**
     * Get the TMX level map
     * @return Reference to the tmx object
     */
    tmx &getLevelMap() { return *levelMap; }
    const tmx &getLevelMap() const { return *levelMap; }

    /**
     * Get the NavMesh
     * @return Reference to the NavMesh object
     */
    NavMesh &getNavMesh() { return *navmesh; }
    const NavMesh &getNavMesh() const { return *navmesh; }

    /**
     * Get the entities data file
     * @return Reference to the entities DataFile
     */
    DataFile &getEntities() { return entities; }
    const DataFile &getEntities() const { return entities; }

    /**
     * Get the details data file
     * @return Reference to the details DataFile
     */
    DataFile &getDetails() { return details; }
    const DataFile &getDetails() const { return details; }

    /**
     * Get tile width from the TMX map
     * @return Tile width in pixels
     */
    int getTileWidth() const { return tileWidth; }

    /**
     * Get tile height from the TMX map
     * @return Tile height in pixels
     */
    int getTileHeight() const { return tileHeight; }

    /**
     * Render all layers of the level map
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     * @param worldX World X offset
     * @param worldY World Y offset
     */
    void render(const CFNativeCamera &camera, const DataFile &config, float worldX = 0.0f, float worldY = 0.0f);

    /**
     * Debug print level information
     */
    void debugPrint() const;
};
