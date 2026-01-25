#pragma once

#include "LevelV1.h"
#include <unordered_map>
#include <string>

/**
 * LevelV2 - Extended level loader and manager class
 *
 * Extends LevelV1 with additional features and improvements.
 * Adds UUID-based agent tracking for more efficient lookups.
 */
class LevelV2 : public LevelV1
{
private:
    // Map of UUID to agent pointer for fast lookups
    std::unordered_map<std::string, AnimatedDataCharacterNavMeshAgent *> agentsByUuid;

    // Map of agent pointer to UUID for reverse lookups
    std::unordered_map<AnimatedDataCharacterNavMeshAgent *, std::string> uuidsByAgent;

    // Counter for generating sequential UUIDs
    size_t nextUuidCounter;

    /**
     * Generate a new UUID for an agent
     * @return A unique UUID string
     */
    std::string generateUuid();

public:
    /**
     * Constructor - initializes all level components from a directory
     * @param directoryPath Path to the level directory (e.g., "/assets/Levels/test_two")
     */
    explicit LevelV2(const std::string &directoryPath);

    /**
     * Destructor
     */
    ~LevelV2() = default;

    /**
     * Add a NavMesh agent to the level (overrides LevelV1)
     * Also adds agent to UUID map for fast lookups
     * @param agent Unique pointer to the agent to add
     * @return Raw pointer to the added agent (for reference, level owns the agent)
     */
    AnimatedDataCharacterNavMeshAgent *addAgent(std::unique_ptr<AnimatedDataCharacterNavMeshAgent> agent) override;

    /**
     * Remove all agents from the level (overrides LevelV1)
     * Also clears the UUID map
     */
    void clearAgents() override;

    /**
     * Get an agent by UUID
     * @param uuid The UUID of the agent to retrieve
     * @return Pointer to the agent, or nullptr if not found
     */
    AnimatedDataCharacterNavMeshAgent *getAgentByUuid(const std::string &uuid);
    const AnimatedDataCharacterNavMeshAgent *getAgentByUuid(const std::string &uuid) const;

    /**
     * Get the UUID for an agent
     * @param agent Pointer to the agent
     * @return The UUID string, or empty string if agent not found
     */
    std::string getUuidForAgent(const AnimatedDataCharacterNavMeshAgent *agent) const;
};
