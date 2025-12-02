#include "LevelV1.h"
#include "CFNativeCamera.h"
#include "JobSystem.h"
#include <cstdio>
#include "DebugPrint.h"

LevelV1::LevelV1(const std::string &directoryPath)
    : levelDirectory(directoryPath), levelName(""), levelMap(nullptr), navmesh(nullptr), entities(), details(), tileWidth(0), tileHeight(0), initialized(false)
{
    DebugPrint::Print("Level", "LevelV1: Loading level from directory: %s\n", directoryPath.c_str());

    // Extract level name from directory path (last component)
    size_t lastSlash = directoryPath.find_last_of('/');
    if (lastSlash != std::string::npos)
    {
        levelName = directoryPath.substr(lastSlash + 1);
    }
    else
    {
        levelName = directoryPath;
    }

    // Load details.json first to get level name if available
    std::string detailsPath = directoryPath + "/details.json";
    try
    {
        details = DataFile(detailsPath);
        DebugPrint::Print("Level", "LevelV1: Loaded details from: %s\n", detailsPath.c_str());

        // Override level name if present in details
        if (details.contains("name"))
        {
            levelName = details["name"].get<std::string>();
            DebugPrint::Print("Level", "LevelV1: Level name from details: %s\n", levelName.c_str());
        }
    }
    catch (const std::exception &e)
    {
        DebugPrint::Print("Level", "LevelV1 Warning: Could not load details.json: %s\n", e.what());
        DebugPrint::Print("Level", "LevelV1: Using extracted directory name: %s\n", levelName.c_str());
    }

    // Load entities.json
    std::string entitiesPath = directoryPath + "/entities.json";
    try
    {
        entities = DataFile(entitiesPath);
        DebugPrint::Print("Level", "LevelV1: Loaded entities from: %s\n", entitiesPath.c_str());
    }
    catch (const std::exception &e)
    {
        DebugPrint::Print("Level", "LevelV1 Warning: Could not load entities.json: %s\n", e.what());
        // Initialize with empty entities structure
        entities = DataFile();
        entities["entities"] = nlohmann::json::array();
    }

    // Load TMX level map
    std::string tmxPath = directoryPath + "/" + levelName + ".tmx";
    try
    {
        levelMap = std::make_unique<tmx>(tmxPath);
        DebugPrint::Print("Level", "LevelV1: Loaded TMX map from: %s\n", tmxPath.c_str());

        // Cache tile dimensions
        tileWidth = levelMap->getTileWidth();
        tileHeight = levelMap->getTileHeight();
        DebugPrint::Print("Level", "LevelV1: Tile dimensions: %dx%d\n", tileWidth, tileHeight);

        // Debug print TMX info
        levelMap->debugPrint();
    }
    catch (const std::exception &e)
    {
        DebugPrint::Print("Level", "LevelV1 Error: Could not load TMX map from %s: %s\n", tmxPath.c_str(), e.what());
        return;
    }

    // Initialize NavMesh
    navmesh = std::make_unique<NavMesh>();

    // Build NavMesh from TMX navmesh layer if available
    if (levelMap->getNavMeshLayerCount() > 0)
    {
        auto navLayer = levelMap->getNavMeshLayer(0);
        DebugPrint::Print("Level", "LevelV1: Building navmesh from layer: %s\n", navLayer->name.c_str());

        navmesh->buildFromLayer(navLayer, tileWidth, tileHeight, 0.0f, 0.0f, false);
        DebugPrint::Print("Level", "LevelV1: NavMesh created with %d polygons\n", navmesh->getPolygonCount());
    }
    else
    {
        DebugPrint::Print("Level", "LevelV1 Warning: No navmesh layers found in level. Navigation mesh not created.\n");
    }

    // Create agents from entities.json
    if (entities.contains("entities") && entities["entities"].is_array())
    {
        DebugPrint::Print("Level", "LevelV1: Creating agents from entities.json...\n");

        for (const auto &entityEntry : entities["entities"])
        {
            // Check if entity has required fields
            if (!entityEntry.contains("datafilePath"))
            {
                DebugPrint::Print("Level", "LevelV1 Warning: Entity missing 'datafilePath' field, skipping\n");
                continue;
            }

            std::string datafilePath = entityEntry["datafilePath"].get<std::string>();
            std::string entityName = entityEntry.contains("name") ? entityEntry["name"].get<std::string>() : "unnamed";

            DebugPrint::Print("Level", "LevelV1: Creating agent '%s' from: %s\n", entityName.c_str(), datafilePath.c_str());

            // Create the agent
            auto agent = createAgentFromFile(datafilePath);

            if (agent)
            {
                // Set position if provided (position is in tile coordinates)
                if (entityEntry.contains("position"))
                {
                    const auto &pos = entityEntry["position"];
                    if (pos.contains("x") && pos.contains("y"))
                    {
                        // Get tile coordinates
                        float tileX = pos["x"].get<float>();
                        float tileY = pos["y"].get<float>();

                        // Convert tile coordinates to world pixel coordinates
                        // Multiply by tile dimensions to get world position
                        float worldX = tileX * tileWidth;
                        float worldY = tileY * tileHeight;

                        agent->setPosition(cf_v2(worldX, worldY));
                        DebugPrint::Print("Level", "LevelV1:   Set agent position to tile (%.1f, %.1f) = world (%.1f, %.1f)\n",
                                          tileX, tileY, worldX, worldY);
                    }
                }

                DebugPrint::Print("Level", "LevelV1:   Agent '%s' created successfully\n", entityName.c_str());
            }
            else
            {
                DebugPrint::Print("Level", "LevelV1 Error: Failed to create agent '%s'\n", entityName.c_str());
            }
        }

        DebugPrint::Print("Level", "LevelV1: Created %zu agents from entities.json\n", agents.size());
    }
    else
    {
        DebugPrint::Print("Level", "LevelV1: No entities array found in entities.json\n");
    }

    // Mark as successfully initialized
    initialized = true;
    DebugPrint::Print("Level", "LevelV1: Level '%s' initialized successfully\n", levelName.c_str());
}

AnimatedDataCharacterNavMeshAgent *LevelV1::addAgent(std::unique_ptr<AnimatedDataCharacterNavMeshAgent> agent)
{
    if (!agent)
    {
        DebugPrint::Print("Level", "LevelV1 Warning: Attempted to add null agent\n");
        return nullptr;
    }

    // Set the agent's navmesh to this level's navmesh
    if (navmesh)
    {
        agent->setNavMesh(navmesh.get());
    }

    agents.push_back(std::move(agent));
    DebugPrint::Print("Level", "LevelV1: Added agent (total: %zu)\n", agents.size());

    return agents.back().get();
}

AnimatedDataCharacterNavMeshAgent *LevelV1::createAgentFromFile(const std::string &entityDataPath)
{
    auto agent = std::make_unique<AnimatedDataCharacterNavMeshAgent>();

    if (!agent->init(entityDataPath))
    {
        DebugPrint::Print("Level", "LevelV1 Error: Failed to initialize agent from: %s\n", entityDataPath.c_str());
        return nullptr;
    }

    DebugPrint::Print("Level", "LevelV1: Created agent from: %s\n", entityDataPath.c_str());
    return addAgent(std::move(agent));
}

AnimatedDataCharacterNavMeshAgent *LevelV1::getAgent(size_t index)
{
    if (index >= agents.size())
    {
        DebugPrint::Print("Level", "LevelV1 Warning: Agent index %zu out of bounds (size: %zu)\n", index, agents.size());
        return nullptr;
    }

    return agents[index].get();
}

const AnimatedDataCharacterNavMeshAgent *LevelV1::getAgent(size_t index) const
{
    if (index >= agents.size())
    {
        DebugPrint::Print("Level", "LevelV1 Warning: Agent index %zu out of bounds (size: %zu)\n", index, agents.size());
        return nullptr;
    }

    return agents[index].get();
}

void LevelV1::clearAgents()
{
    agents.clear();
    DebugPrint::Print("Level", "LevelV1: Cleared all agents\n");
}

void LevelV1::updateAgents(float dt)
{

    // TODO this!!!!!!
    // all of this is assuming agents are on screen, todo cull based on camera and do different for offscreen agents
    // Update agents with move vectors (using results from completed background jobs)
    for (auto &agent : agents)
    {
        if (agent)
        {
            // Get the background move vector if the job is complete
            v2 moveVector = cf_v2(0.0f, 0.0f);
            if (agent->isBackgroundUpdateComplete())
            {
                moveVector = agent->getBackgroundMoveVector();
            }

            agent->update(dt, moveVector);
        }
    }
    // trigger background updates for all agents
    // these will finish on their own and update the agent as needed
    for (auto &agent : agents)
    {
        if (agent)
        {
            // Try to start a background job for this agent
            agent->backgroundUpdate(dt, true); // TODO change to false if offscreen
        }
    }

    // Kick off all pending jobs (non-blocking)
    JobSystem::kick();
}

void LevelV1::renderAgents(const CFNativeCamera &camera)
{
    // Get camera view bounds for culling
    CF_Aabb viewBounds = camera.getViewBounds();

    // Reasonable default size for agent bounds (can be adjusted or made configurable)
    const float agentHalfSize = 32.0f; // Assumes agent is roughly 64x64 pixels

    int renderedCount = 0;
    int culledCount = 0;

    for (auto &agent : agents)
    {
        if (agent)
        {
            v2 agentPos = agent->getPosition();

            // Create a bounding box around the agent
            CF_Aabb agentBounds = make_aabb(
                cf_v2(agentPos.x - agentHalfSize, agentPos.y - agentHalfSize),
                cf_v2(agentPos.x + agentHalfSize, agentPos.y + agentHalfSize));

            // Check if agent is visible in viewport
            if (camera.isVisible(agentBounds))
            {
                agent->render(agentPos);
                renderedCount++;
            }
            else
            {
                culledCount++;
            }
        }
    }

    // Optional: Uncomment for debugging viewport culling
    // if (culledCount > 0)
    // {
    //     printf("LevelV1: Rendered %d agents, culled %d agents\n", renderedCount, culledCount);
    // }
}

void LevelV1::render(const CFNativeCamera &camera, const DataFile &config, float worldX, float worldY)
{
    if (!initialized || !levelMap)
    {
        return;
    }

    // TODO change render order based on height in world, not agents always on top
    levelMap->renderAllLayers(camera, config, worldX, worldY);
    renderAgents(camera);
}

void LevelV1::debugPrint() const
{
    DebugPrint::Print("Level", "=== LevelV1 Debug Info ===\n");
    DebugPrint::Print("Level", "  Directory: %s\n", levelDirectory.c_str());
    DebugPrint::Print("Level", "  Name: %s\n", levelName.c_str());
    DebugPrint::Print("Level", "  Initialized: %s\n", initialized ? "yes" : "no");
    DebugPrint::Print("Level", "  Tile Size: %dx%d\n", tileWidth, tileHeight);

    if (levelMap)
    {
        DebugPrint::Print("Level", "  TMX Map: loaded\n");
    }
    else
    {
        DebugPrint::Print("Level", "  TMX Map: NOT loaded\n");
    }

    if (navmesh)
    {
        DebugPrint::Print("Level", "  NavMesh: %d polygons, %d points\n",
                          navmesh->getPolygonCount(), navmesh->getPointCount());
    }
    else
    {
        DebugPrint::Print("Level", "  NavMesh: NOT created\n");
    }

    DebugPrint::Print("Level", "  Agents: %zu\n", agents.size());
    for (size_t i = 0; i < agents.size(); ++i)
    {
        if (agents[i])
        {
            v2 pos = agents[i]->getPosition();
            DebugPrint::Print("Level", "    Agent %zu: pos=(%.1f, %.1f), polygon=%d, walkable=%s\n",
                              i, pos.x, pos.y,
                              agents[i]->getCurrentPolygon(),
                              agents[i]->isOnWalkableArea() ? "yes" : "no");
        }
    }

    DebugPrint::Print("Level", "  Entities: %s\n", entities.dump(2).c_str());
    DebugPrint::Print("Level", "  Details: %s\n", details.dump(2).c_str());
    DebugPrint::Print("Level", "========================\n");
}
