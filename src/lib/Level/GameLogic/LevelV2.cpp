#include "LevelV2.h"
#include <cstdio>
#include <sstream>
#include <iomanip>

LevelV2::LevelV2(const std::string &directoryPath)
    : LevelV1(directoryPath), nextUuidCounter(0)
{
    printf("LevelV2: Initialized as extension of LevelV1\n");

    // Validate level version
    if (!isInitialized())
    {
        printf("LevelV2 Error: Parent LevelV1 failed to initialize\n");
        return;
    }

    const DataFile &details = getDetails();

    // Check if level_version key exists
    if (!details.contains("level_version"))
    {
        printf("LevelV2 Error: details.json missing required 'level_version' key\n");
        printf("LevelV2: Level initialization failed - invalid version\n");
        setInitialized(false);
        return;
    }

    // Check if level_version is an integer
    if (!details["level_version"].is_number_integer())
    {
        printf("LevelV2 Error: 'level_version' must be an integer, got: %s\n",
               details["level_version"].dump().c_str());
        printf("LevelV2: Level initialization failed - invalid version type\n");
        setInitialized(false);
        return;
    }

    // Check if level_version is 2 or greater
    int levelVersion = details["level_version"].get<int>();
    if (levelVersion < 2)
    {
        printf("LevelV2 Error: 'level_version' must be 2 or greater, got: %d\n", levelVersion);
        printf("LevelV2: Level initialization failed - version too low\n");
        setInitialized(false);
        return;
    }

    printf("LevelV2: Validated level version: %d\n", levelVersion);

    // Build UUID map for all agents that were created by parent constructor
    for (size_t i = 0; i < getAgentCount(); ++i)
    {
        AnimatedDataCharacterNavMeshAgent *agent = getAgent(i);
        if (agent)
        {
            std::string uuid = generateUuid();
            agentsByUuid[uuid] = agent;
            uuidsByAgent[agent] = uuid;
            printf("LevelV2: Indexed agent with UUID: %s\n", uuid.c_str());
        }
    }
    printf("LevelV2: Built UUID index with %zu agents\n", agentsByUuid.size());
}

std::string LevelV2::generateUuid()
{
    std::ostringstream oss;
    oss << "agent_" << std::setfill('0') << std::setw(6) << nextUuidCounter++;
    return oss.str();
}

AnimatedDataCharacterNavMeshAgent *LevelV2::addAgent(std::unique_ptr<AnimatedDataCharacterNavMeshAgent> agent)
{
    if (!agent)
    {
        printf("LevelV2 Warning: Attempted to add null agent\n");
        return nullptr;
    }

    // Call parent class implementation to add to vector and spatial grid
    AnimatedDataCharacterNavMeshAgent *agentPtr = LevelV1::addAgent(std::move(agent));

    if (agentPtr)
    {
        // Generate UUID and add to maps
        std::string uuid = generateUuid();
        agentsByUuid[uuid] = agentPtr;
        uuidsByAgent[agentPtr] = uuid;
        printf("LevelV2: Added agent to UUID map: %s\n", uuid.c_str());
    }

    return agentPtr;
}

void LevelV2::clearAgents()
{
    // Clear UUID maps
    agentsByUuid.clear();
    uuidsByAgent.clear();
    nextUuidCounter = 0;
    printf("LevelV2: Cleared UUID maps\n");

    // Call parent class implementation to clear agents vector and spatial grid
    LevelV1::clearAgents();
}

AnimatedDataCharacterNavMeshAgent *LevelV2::getAgentByUuid(const std::string &uuid)
{
    auto it = agentsByUuid.find(uuid);
    if (it != agentsByUuid.end())
    {
        return it->second;
    }
    return nullptr;
}

const AnimatedDataCharacterNavMeshAgent *LevelV2::getAgentByUuid(const std::string &uuid) const
{
    auto it = agentsByUuid.find(uuid);
    if (it != agentsByUuid.end())
    {
        return it->second;
    }
    return nullptr;
}

std::string LevelV2::getUuidForAgent(const AnimatedDataCharacterNavMeshAgent *agent) const
{
    auto it = uuidsByAgent.find(const_cast<AnimatedDataCharacterNavMeshAgent *>(agent));
    if (it != uuidsByAgent.end())
    {
        return it->second;
    }
    return "";
}
