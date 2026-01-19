#pragma once

#include <string>
#include <memory>
#include <vector>
#include "LevelMap.h"
#include "NavMesh.h"
#include "DataFile.h"
#include "SpatialGrid.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "WorldPositionRenderedObjectsList.h"

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
    std::unique_ptr<LevelMap> levelMap;
    std::unique_ptr<NavMesh> navmesh;

    // Level data files
    DataFile entities;
    DataFile details;

    // NavMesh agents in this level
    std::vector<std::unique_ptr<AnimatedDataCharacterNavMeshAgent>> agents;

    // Spatial partitioning grid for efficient queries
    SpatialGrid spatialGrid;

    // List of all objects to render sorted by world Y position
    WorldPositionRenderedObjectsList renderedObjects;

    // Player character reference for hitbox checking (non-owning)
    const AnimatedDataCharacter *player;

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
     * Get the level map
     * @return Reference to the LevelMap object
     */
    LevelMap &getLevelMap() { return *levelMap; }
    const LevelMap &getLevelMap() const { return *levelMap; }

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
     * Check if a character is inside any active action hitbox in warmup phase
     * @param character The character to check
     * @param characterBox The bounding box of the character
     * @return true if character is inside an action hitbox during warmup
     */
    bool isCharacterInActionHitbox(const AnimatedDataCharacter *character, CF_Aabb characterBox) const;

    /**
     * Get all characters that are inside an action's hitbox
     * @param action The action whose hitbox to check
     * @param excludeCharacter Character to exclude from results (usually the one performing the action)
     * @return Vector of characters inside the action's hitbox
     */
    std::vector<AnimatedDataCharacter *> getCharactersInActionHitbox(const Action *action, const AnimatedDataCharacter *excludeCharacter = nullptr) const;

    /**
     * Set the player character reference for hitbox checking
     * @param player Pointer to the player character
     */
    void setPlayer(const AnimatedDataCharacter *player);

    /**
     * Get all entities (agents) at a specific tile coordinate
     * Uses rendering coordinate system: tile_x=0 is left, tile_y=0 is bottom
     * @param tile_x The x coordinate of the tile (0 = left column)
     * @param tile_y The y coordinate of the tile (0 = bottom row)
     * @return Vector of pointers to agents at that tile location
     */
    std::vector<AnimatedDataCharacterNavMeshAgent *> get_entities_at(int tile_x, int tile_y) const;

    /**
     * Update all agents in the level
     * @param dt Delta time in seconds
     */
    void updateAgents(float dt);

    /**
     * Render all layers of the level map (tiles only)
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     * @param worldX World X offset
     * @param worldY World Y offset
     */
    void renderLayers(const CFNativeCamera &camera, const DataFile &config, float worldX = 0.0f, float worldY = 0.0f);

    /**
     * Render all action hitboxes for agents that are visible in the camera viewport
     * This should be called after renderLayers and before renderAgents
     * @param camera Camera to use for viewport culling
     * @param player Optional player character to also render action hitbox for
     */
    void renderAgentActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player = nullptr);

    /**
     * Render available actions for the player character
     * Highlights tiles where the player can perform actions from pointer A or B slots
     * @param camera Camera to use for rendering
     * @param player Player character to get available actions from
     */
    void renderPlayerAvailableActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player);

    /**
     * Render all agents in the level that are visible in the camera viewport
     * @param camera Camera to use for viewport culling
     */
    void renderAgents(const CFNativeCamera &camera);

    /**
     * Render all layers of the level map, agents, actions, and player
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     * @param player Optional player character to render
     * @param worldX World X offset
     * @param worldY World Y offset
     */
    void render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player = nullptr, float worldX = 0.0f, float worldY = 0.0f);

    /**
     * Debug print level information
     */
    void debugPrint() const;
};
