#pragma once

#include <string>
#include <memory>
#include <vector>
#include "tmx.h"
#include "NavMesh.h"
#include "DataFile.h"
#include "SpatialGrid.h"
#include "AnimatedDataCharacterNavMeshAgent.h"

// Forward declarations
class CFNativeCamera;
class AnimatedDataCharacter;

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

    // NavMesh agents in this level
    std::vector<std::unique_ptr<AnimatedDataCharacterNavMeshAgent>> agents;

    // Spatial partitioning grid for efficient queries
    SpatialGrid spatialGrid;

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
     * Add a NavMesh agent to the level
     * @param agent Unique pointer to the agent to add
     * @return Raw pointer to the added agent (for reference, level owns the agent)
     */
    AnimatedDataCharacterNavMeshAgent *addAgent(std::unique_ptr<AnimatedDataCharacterNavMeshAgent> agent);

    /**
     * Create and add a NavMesh agent from an entity data file
     * @param entityDataPath Path to the entity JSON file
     * @return Raw pointer to the created agent, or nullptr if creation failed
     */
    AnimatedDataCharacterNavMeshAgent *createAgentFromFile(const std::string &entityDataPath);

    /**
     * Get the number of agents in the level
     * @return Number of agents
     */
    size_t getAgentCount() const { return agents.size(); }

    /**
     * Get the spatial grid for efficient spatial queries
     * @return Reference to the SpatialGrid
     */
    SpatialGrid &getSpatialGrid() { return spatialGrid; }
    const SpatialGrid &getSpatialGrid() const { return spatialGrid; }

    /**
     * Update the spatial grid with current agent positions
     * Call this after agents have moved
     */
    void updateSpatialGrid();

    /**
     * Rebuild the entire spatial grid from scratch
     * Use when agents have been added/removed or after significant changes
     */
    void rebuildSpatialGrid();

    /**
     * Get an agent by index
     * @param index Index of the agent
     * @return Pointer to the agent, or nullptr if index is out of bounds
     */
    AnimatedDataCharacterNavMeshAgent *getAgent(size_t index);
    const AnimatedDataCharacterNavMeshAgent *getAgent(size_t index) const;

    /**
     * Remove all agents from the level
     */
    void clearAgents();

    /**
     * Check if any agents (excluding the specified agent) are within a given area
     * @param area The AABB area to check
     * @param excludeAgent Agent to exclude from the check (usually the one doing the checking)
     * @return true if any other agents are found in the area, false otherwise
     */
    bool checkAgentsInArea(const std::vector<CF_Aabb> &areas, CF_Aabb areasBounds, const AnimatedDataCharacter *excludeAgent = nullptr) const;

    /**
     * Update all agents in the level
     * @param dt Delta time in seconds
     */
    void updateAgents(float dt);

    /**
     * Render all agents in the level that are visible in the camera viewport
     * @param camera Camera to use for viewport culling
     */
    void renderAgents(const CFNativeCamera &camera);

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
