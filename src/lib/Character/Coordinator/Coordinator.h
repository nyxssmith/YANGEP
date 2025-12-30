#pragma once

#include <vector>
#include <mutex>
#include <unordered_set>

// Forward declaration
class AnimatedDataCharacterNavMeshAgent;

// Coordinator class manages on-screen agents that need coordination
// Thread-safe for use with background workers
class Coordinator
{
public:
    Coordinator();
    ~Coordinator();

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

private:
    std::vector<AnimatedDataCharacterNavMeshAgent *> m_agents;
    std::unordered_set<AnimatedDataCharacterNavMeshAgent *> m_agentSet; // For O(1) lookup
    mutable std::mutex m_mutex;                                         // Protects m_agents and m_agentSet
};
