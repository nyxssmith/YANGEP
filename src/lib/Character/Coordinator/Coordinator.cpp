#include "Coordinator.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include <algorithm>
#include <stdio.h>

Coordinator::Coordinator()
{
    printf("Coordinator: Initialized\n");
}

Coordinator::~Coordinator()
{
    clear();
    printf("Coordinator: Destroyed\n");
}

void Coordinator::addAgent(AnimatedDataCharacterNavMeshAgent *agent)
{
    if (!agent)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if agent is already in the set (O(1) lookup)
    if (m_agentSet.find(agent) != m_agentSet.end())
    {
        return; // Already coordinating this agent
    }

    // Add to both set and vector
    m_agentSet.insert(agent);
    m_agents.push_back(agent);
}

void Coordinator::removeAgent(AnimatedDataCharacterNavMeshAgent *agent)
{
    if (!agent)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if agent is in the set
    auto it = m_agentSet.find(agent);
    if (it == m_agentSet.end())
    {
        return; // Agent not being coordinated
    }

    // Remove from set
    m_agentSet.erase(it);

    // Remove from vector (preserve order not critical, so swap-and-pop for efficiency)
    auto vecIt = std::find(m_agents.begin(), m_agents.end(), agent);
    if (vecIt != m_agents.end())
    {
        // Swap with last element and pop
        if (vecIt != m_agents.end() - 1)
        {
            std::swap(*vecIt, m_agents.back());
        }
        m_agents.pop_back();
    }
}

std::vector<AnimatedDataCharacterNavMeshAgent *> Coordinator::getAgents() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_agents; // Return a copy
}

size_t Coordinator::getAgentCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_agents.size();
}

void Coordinator::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_agents.clear();
    m_agentSet.clear();
}
