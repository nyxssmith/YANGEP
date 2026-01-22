#pragma once

#include <vector>
#include <mutex>
#include <unordered_set>
#include <chrono>
#include <cute.h>
#include "NearPlayerTileGrid.h"
#include "HitBox.h" // For HitboxTile

using namespace Cute;

// Forward declarations
class AnimatedDataCharacterNavMeshAgent;
class AnimatedDataCharacter;
class LevelV1;
class HitBox;
class Action;

// Structure to hold safe copies of agent data for processing
struct AgentProcessData
{
    AnimatedDataCharacterNavMeshAgent *agent; // Pointer for identification only
    v2 position;
    std::vector<HitboxTile> hitboxTiles; // Copied tiles data
    bool hasValidAction;
    float distSq;
};

// Coordinator class manages on-screen agents that need coordination
// Thread-safe for use with background workers
class Coordinator
{
public:
    Coordinator();
    ~Coordinator();

    // Initialize the coordinator with player and level pointers
    void initialize(const AnimatedDataCharacter *player, LevelV1 *level);

    // Add an agent to coordination (if not already added)
    // Thread-safe
    void addAgent(AnimatedDataCharacterNavMeshAgent *agent);

    // Remove an agent from coordination
    // Thread-safe
    void removeAgent(AnimatedDataCharacterNavMeshAgent *agent);

    // Get all agents currently being coordinated
    // Returns a copy for thread safety
    std::vector<AnimatedDataCharacterNavMeshAgent *> getAgents() const;

    // Get the number of agents being coordinated
    size_t getAgentCount() const;

    // Clear all agents from coordination
    void clear();

    // Update all coordinated agents
    // Thread-safe - acquires mutex and holds it during entire update
    // Note: Agents should not be deleted by other threads during update
    void update();

    // Get the near-player tile grid
    const NearPlayerTileGrid &getNearPlayerTileGrid() const;

    // Get the near-player tile grid (mutable)
    NearPlayerTileGrid &getNearPlayerTileGridMutable();

    // Set the near-player tile grid size (reinitializes the grid)
    void setNearPlayerTileGridSize(int gridSize);

    // Get the player pointer
    const AnimatedDataCharacter *getPlayer() const;

    // Get the level pointer
    LevelV1 *getLevel() const;

    // Get the last update execution time in milliseconds
    double getLastUpdateTimeMs() const;

    // Update the near-player grid based on player's current tile position
    // Thread-safe
    void updateNearPlayerGrid(int playerTileX, int playerTileY);

    // Render the coordinator's near-player grid (for debugging)
    void render() const;

private:
    // Try to place hitbox tiles on the grid using copied data
    // Returns true if placement was successful
    bool tryPlaceHitboxOnGrid(const std::vector<HitboxTile> &hitboxTiles, v2 agentPosition, AnimatedDataCharacterNavMeshAgent *agent);

    std::vector<AnimatedDataCharacterNavMeshAgent *> m_agents;
    std::unordered_set<AnimatedDataCharacterNavMeshAgent *> m_agentSet; // For O(1) lookup
    NearPlayerTileGrid m_nearPlayerTileGrid;                            // Grid of tiles around the player
    const AnimatedDataCharacter *m_player;                              // Non-owning pointer to the player
    LevelV1 *m_level;                                                   // Non-owning pointer to the level
    int m_lastPlayerTileX;                                              // Last known player tile X position
    int m_lastPlayerTileY;                                              // Last known player tile Y position
    bool m_agentListChanged;                                            // Flag to track if agents were added/removed
    double m_lastUpdateTimeMs;                                          // Last update execution time in milliseconds
    mutable std::mutex m_mutex;                                         // Protects m_agents and m_agentSet
};
