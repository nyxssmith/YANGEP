#pragma once

#include <string>
#include <vector>
#include <memory>
#include "LevelV2.h"
#include "DataFile.h"
#include "SpatialGrid.h"
#include "NavMesh.h"

/**
 * WorldLevel - Container for a level within a world
 *
 * Holds a LevelV2 instance along with its world position and visibility state
 */
struct WorldLevel
{
    std::unique_ptr<LevelV2> level; // The level instance
    std::string folderPath;         // Path to the level directory
    float xOffset;                  // X offset in world coordinates
    float yOffset;                  // Y offset in world coordinates
    bool isOnScreen;                // Whether this level is currently visible

    /**
     * Constructor
     * @param path Folder path to the level
     * @param x X offset in world coordinates
     * @param y Y offset in world coordinates
     */
    WorldLevel(const std::string &path, float x, float y);
};

/**
 * World - A collection of levels that make up a game world
 *
 * Loads and manages multiple levels from a world directory.
 * Reads levels.json to determine which levels to load and their positions.
 */
class World
{
private:
    std::string worldDirectory;
    std::vector<WorldLevel> levels;
    bool initialized;

    // World-level spatial partitioning grid for all agents
    SpatialGrid spatialGrid;

    // World-level combined navmesh from all levels
    std::unique_ptr<NavMesh> navmesh;

public:
    /**
     * Constructor - loads all levels from a world directory
     * @param directoryPath Path to the world directory (e.g., "/assets/Worlds/test-a")
     */
    explicit World(const std::string &directoryPath);

    /**
     * Destructor
     */
    ~World() = default;

    /**
     * Check if the world was successfully initialized
     * @return true if all levels loaded successfully, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * Get the world directory path
     * @return World directory string
     */
    const std::string &getWorldDirectory() const { return worldDirectory; }

    /**
     * Get the number of levels in the world
     * @return Number of levels
     */
    size_t getLevelCount() const { return levels.size(); }

    /**
     * Get a level by index
     * @param index Index of the level
     * @return Reference to the WorldLevel, throws if index is out of bounds
     */
    WorldLevel &getLevel(size_t index);
    const WorldLevel &getLevel(size_t index) const;

    /**
     * Get all levels
     * @return Reference to the levels vector
     */
    std::vector<WorldLevel> &getLevels() { return levels; }
    const std::vector<WorldLevel> &getLevels() const { return levels; }

    /**
     * Update visibility flags for all levels based on camera viewport
     * @param viewBounds The camera's view bounds
     */
    void updateLevelVisibility(const CF_Aabb &viewBounds);

    /**
     * Get the world's combined NavMesh
     * @return Reference to the NavMesh object
     */
    NavMesh &getNavMesh() { return *navmesh; }
    const NavMesh &getNavMesh() const { return *navmesh; }

    /**
     * Get the world's spatial grid
     * @return Reference to the SpatialGrid
     */
    SpatialGrid &getSpatialGrid() { return spatialGrid; }
    const SpatialGrid &getSpatialGrid() const { return spatialGrid; }

    // Agent management methods

    /**
     * Get total number of agents across all levels
     * @return Total agent count
     */
    size_t getAgentCount() const;

    /**
     * Get number of agents in onscreen levels
     * @return Onscreen agent count
     */
    size_t getOnscreenAgentCount() const;

    /**
     * Get number of agents in offscreen levels
     * @return Offscreen agent count
     */
    size_t getOffscreenAgentCount() const;

    /**
     * Get all agents from all levels
     * @return Vector of agent pointers
     */
    std::vector<AnimatedDataCharacterNavMeshAgent *> getAgents();
    std::vector<const AnimatedDataCharacterNavMeshAgent *> getAgents() const;

    /**
     * Get agents from onscreen levels only
     * @return Vector of onscreen agent pointers
     */
    std::vector<AnimatedDataCharacterNavMeshAgent *> getOnscreenAgents();
    std::vector<const AnimatedDataCharacterNavMeshAgent *> getOnscreenAgents() const;

    /**
     * Get agents from offscreen levels only
     * @return Vector of offscreen agent pointers
     */
    std::vector<AnimatedDataCharacterNavMeshAgent *> getOffscreenAgents();
    std::vector<const AnimatedDataCharacterNavMeshAgent *> getOffscreenAgents() const;

    /**
     * Set player character reference for all levels
     * @param player Pointer to the player character
     */
    void setPlayer(const AnimatedDataCharacter *player);

    /**
     * Clear all agents from all levels
     */
    void clearAgents();

    /**
     * Clear agents from onscreen levels only
     */
    void clearOnscreenAgents();

    /**
     * Clear agents from offscreen levels only
     */
    void clearOffscreenAgents();

    // Update methods

    /**
     * Update all agents in all levels
     * @param dt Delta time in seconds
     */
    void updateAgents(float dt);

    /**
     * Update agents in onscreen levels only
     * @param dt Delta time in seconds
     */
    void updateOnscreenAgents(float dt);

    /**
     * Update agents in offscreen levels only
     * @param dt Delta time in seconds
     */
    void updateOffscreenAgents(float dt);

    /**
     * Update the world's spatial grid with current agent positions
     * Call this after agents have moved
     */
    void updateSpatialGrid();

    /**
     * Rebuild the world's spatial grid from scratch
     * Use when agents have been added/removed or after significant changes
     */
    void rebuildSpatialGrid();

    /**
     * Set all dying agents to dead state in all levels
     */
    void cullDyingAgents();

    /**
     * Set all dying agents to dead state in onscreen levels only
     */
    void cullOnscreenDyingAgents();

    /**
     * Set all dying agents to dead state in offscreen levels only
     */
    void cullOffscreenDyingAgents();

    // Rendering methods

    /**
     * Render all layers for all onscreen levels
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     */
    void renderLayers(const CFNativeCamera &camera, const DataFile &config);

    /**
     * Render action hitboxes for all onscreen levels
     * @param camera Camera to use for viewport culling
     * @param player Optional player character to also render action hitbox for
     */
    void renderAgentActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player = nullptr);

    /**
     * Render available actions for player in all onscreen levels
     * @param camera Camera to use for rendering
     * @param player Player character to get available actions from
     */
    void renderPlayerAvailableActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player);

    /**
     * Render all agents in all onscreen levels
     * @param camera Camera to use for viewport culling
     */
    void renderAgents(const CFNativeCamera &camera);

    /**
     * Render everything for all onscreen levels
     * @param camera Camera to use for rendering
     * @param config Configuration data file for layer highlighting
     * @param player Optional player character to render
     */
    void render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player = nullptr);

    /**
     * Debug print world information
     */
    void debugPrint() const;
};
