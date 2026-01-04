#include "LevelV1.h"
#include "CFNativeCamera.h"
#include "JobSystem.h"
#include "../UI/ColorUtils.h"
#include <cstdio>

// LevelV1 implementation
LevelV1::LevelV1(const std::string &directoryPath)
    : levelDirectory(directoryPath), levelName(""), levelMap(nullptr), navmesh(nullptr), entities(), details(), tileWidth(0), tileHeight(0), initialized(false), player(nullptr)
{
    printf("LevelV1: Loading level from directory: %s\n", directoryPath.c_str());

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
        printf("LevelV1: Loaded details from: %s\n", detailsPath.c_str());

        // Override level name if present in details
        if (details.contains("name"))
        {
            levelName = details["name"].get<std::string>();
            printf("LevelV1: Level name from details: %s\n", levelName.c_str());
        }
    }
    catch (const std::exception &e)
    {
        printf("LevelV1 Warning: Could not load details.json: %s\n", e.what());
        printf("LevelV1: Using extracted directory name: %s\n", levelName.c_str());
    }

    // Load entities.json
    std::string entitiesPath = directoryPath + "/entities.json";
    try
    {
        entities = DataFile(entitiesPath);
        printf("LevelV1: Loaded entities from: %s\n", entitiesPath.c_str());
    }
    catch (const std::exception &e)
    {
        printf("LevelV1 Warning: Could not load entities.json: %s\n", e.what());
        // Initialize with empty entities structure
        entities = DataFile();
        entities["entities"] = nlohmann::json::array();
    }

    // Load TMX level map
    std::string tmxPath = directoryPath + "/" + levelName + ".tmx";
    try
    {
        levelMap = std::make_unique<LevelMap>(tmxPath);
        printf("LevelV1: Loaded TMX map from: %s\n", tmxPath.c_str());

        // Cache tile dimensions
        tileWidth = levelMap->getTileWidth();
        tileHeight = levelMap->getTileHeight();
        printf("LevelV1: Tile dimensions: %dx%d\n", tileWidth, tileHeight);

        // Debug print TMX info
        levelMap->debugPrint();
    }
    catch (const std::exception &e)
    {
        printf("LevelV1 Error: Could not load TMX map from %s: %s\n", tmxPath.c_str(), e.what());
        return;
    }

    // Initialize NavMesh
    navmesh = std::make_unique<NavMesh>();

    // Build NavMesh from TMX navmesh layer if available
    if (levelMap->getNavMeshLayerCount() > 0)
    {
        auto navLayer = levelMap->getNavMeshLayer(0);
        printf("LevelV1: Building navmesh from layer: %s\n", navLayer->name.c_str());

        navmesh->buildFromLayer(navLayer, tileWidth, tileHeight, 0.0f, 0.0f, false);
        printf("LevelV1: NavMesh created with %d polygons\n", navmesh->getPolygonCount());
    }
    else
    {
        printf("LevelV1 Warning: No navmesh layers found in level. Navigation mesh not created.\n");
    }

    // Apply navmesh cuts from cut layers
    if (navmesh->getPolygonCount() > 0)
    {
        int total_cuts = 0;

        // Process bottom edge cuts
        for (const auto &cutLayer : levelMap->getCutBottomLayers())
        {
            printf("LevelV1: Processing cut layer (bottom): %s\n", cutLayer->name.c_str());
            for (int y = 0; y < cutLayer->height; y++)
            {
                for (int x = 0; x < cutLayer->width; x++)
                {
                    int gid = cutLayer->getTileGID(x, y);
                    if (gid != 0) // Tile is marked for cut
                    {
                        navmesh->applyCut(x, y, NAV_CUT_EDGE_BOTTOM);
                        total_cuts++;
                    }
                }
            }
        }

        // Process top edge cuts
        for (const auto &cutLayer : levelMap->getCutTopLayers())
        {
            printf("LevelV1: Processing cut layer (top): %s\n", cutLayer->name.c_str());
            for (int y = 0; y < cutLayer->height; y++)
            {
                for (int x = 0; x < cutLayer->width; x++)
                {
                    int gid = cutLayer->getTileGID(x, y);
                    if (gid != 0)
                    {
                        navmesh->applyCut(x, y, NAV_CUT_EDGE_TOP);
                        total_cuts++;
                    }
                }
            }
        }

        // Process right edge cuts
        for (const auto &cutLayer : levelMap->getCutRightLayers())
        {
            printf("LevelV1: Processing cut layer (right): %s\n", cutLayer->name.c_str());
            for (int y = 0; y < cutLayer->height; y++)
            {
                for (int x = 0; x < cutLayer->width; x++)
                {
                    int gid = cutLayer->getTileGID(x, y);
                    if (gid != 0)
                    {
                        navmesh->applyCut(x, y, NAV_CUT_EDGE_RIGHT);
                        total_cuts++;
                    }
                }
            }
        }

        // Process left edge cuts
        for (const auto &cutLayer : levelMap->getCutLeftLayers())
        {
            printf("LevelV1: Processing cut layer (left): %s\n", cutLayer->name.c_str());
            for (int y = 0; y < cutLayer->height; y++)
            {
                for (int x = 0; x < cutLayer->width; x++)
                {
                    int gid = cutLayer->getTileGID(x, y);
                    if (gid != 0)
                    {
                        navmesh->applyCut(x, y, NAV_CUT_EDGE_LEFT);
                        total_cuts++;
                    }
                }
            }
        }

        printf("LevelV1: Applied %d navmesh cuts\n", total_cuts);
    }

    // Create agents from entities.json
    if (entities.contains("entities") && entities["entities"].is_array())
    {
        printf("LevelV1: Creating agents from entities.json...\n");

        for (const auto &entityEntry : entities["entities"])
        {
            // Check if entity has required fields
            if (!entityEntry.contains("path"))
            {
                printf("LevelV1 Warning: Entity missing 'path' field, skipping\n");
                continue;
            }

            std::string entityPath = entityEntry["path"].get<std::string>();
            std::string entityName = entityEntry.contains("name") ? entityEntry["name"].get<std::string>() : "unnamed";

            printf("LevelV1: Creating agent '%s' from: %s\n", entityName.c_str(), entityPath.c_str());

            // Create the agent
            auto agent = createAgentFromFile(entityPath);

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
                        printf("LevelV1:   Set agent position to tile (%.1f, %.1f) = world (%.1f, %.1f)\n",
                               tileX, tileY, worldX, worldY);
                    }
                }

                printf("LevelV1:   Agent '%s' created successfully\n", entityName.c_str());
            }
            else
            {
                printf("LevelV1 Error: Failed to create agent '%s'\n", entityName.c_str());
            }
        }

        printf("LevelV1: Created %zu agents from entities.json\n", agents.size());
    }
    else
    {
        printf("LevelV1: No entities array found in entities.json\n");
    }

    // Mark as successfully initialized
    initialized = true;
    printf("LevelV1: Level '%s' initialized successfully\n", levelName.c_str());

    // Build initial spatial grid with all agents
    rebuildSpatialGrid();

    // Add all structures to the rendered objects list
    // Calculate their worldY positions once since they never move
    for (size_t i = 0; i < levelMap->getStructureCount(); ++i)
    {
        auto structure = levelMap->getStructure(i);
        if (structure)
        {
            ObjectRenderedByWorldPosition structureObj(structure.get());

            // Calculate worldY for this structure (only done once)
            float minWorldY = 999999.0f;
            bool foundTile = false;

            for (int y = 0; y < structure->height; y++)
            {
                for (int x = 0; x < structure->width; x++)
                {
                    int gid = structure->getTileGID(x, y);
                    if (gid != 0) // Non-empty tile
                    {
                        // Convert tile coordinates to world coordinates
                        float worldY = ((structure->height - 1 - y) * tileHeight);
                        if (worldY < minWorldY)
                        {
                            minWorldY = worldY;
                        }
                        foundTile = true;
                    }
                }
            }

            if (foundTile)
            {
                structureObj.setWorldY(minWorldY);
            }
            else
            {
                structureObj.setWorldY(0.0f);
            }

            renderedObjects.add(structureObj);
        }
    }

    // Add all agents to the rendered objects list
    for (auto &agent : agents)
    {
        if (agent)
        {
            renderedObjects.add(ObjectRenderedByWorldPosition(agent.get()));
        }
    }

    printf("LevelV1: Added %zu objects to rendered objects list\n", renderedObjects.getCount());
}

AnimatedDataCharacterNavMeshAgent *LevelV1::addAgent(std::unique_ptr<AnimatedDataCharacterNavMeshAgent> agent)
{
    if (!agent)
    {
        printf("LevelV1 Warning: Attempted to add null agent\n");
        return nullptr;
    }

    // Set the agent's navmesh to this level's navmesh
    if (navmesh)
    {
        agent->setNavMesh(navmesh.get());
    }

    // Set the agent's level pointer so it can query other agents
    agent->setLevel(this);

    agents.push_back(std::move(agent));

    // Add to spatial grid
    size_t agentIndex = agents.size() - 1;
    v2 agentPos = agents.back()->getPosition();
    spatialGrid.insert(agentIndex, agentPos, 32.0f);

    // Add to rendered objects list
    renderedObjects.add(ObjectRenderedByWorldPosition(agents.back().get()));

    printf("LevelV1: Added agent (total: %zu, rendered objects: %zu)\n", agents.size(), renderedObjects.getCount());

    return agents.back().get();
}

AnimatedDataCharacterNavMeshAgent *LevelV1::createAgentFromFile(const std::string &entityDataPath)
{
    auto agent = std::make_unique<AnimatedDataCharacterNavMeshAgent>();

    if (!agent->init(entityDataPath))
    {
        printf("LevelV1 Error: Failed to initialize agent from: %s\n", entityDataPath.c_str());
        return nullptr;
    }

    printf("LevelV1: Created agent from: %s\n", entityDataPath.c_str());
    return addAgent(std::move(agent));
}

AnimatedDataCharacterNavMeshAgent *LevelV1::getAgent(size_t index)
{
    if (index >= agents.size())
    {
        printf("LevelV1 Warning: Agent index %zu out of bounds (size: %zu)\n", index, agents.size());
        return nullptr;
    }

    return agents[index].get();
}

const AnimatedDataCharacterNavMeshAgent *LevelV1::getAgent(size_t index) const
{
    if (index >= agents.size())
    {
        printf("LevelV1 Warning: Agent index %zu out of bounds (size: %zu)\n", index, agents.size());
        return nullptr;
    }

    return agents[index].get();
}

void LevelV1::clearAgents()
{
    agents.clear();
    spatialGrid.clear();
    printf("LevelV1: Cleared all agents\n");
}

void LevelV1::updateAgents(float dt)
{

    // TODO this!!!!!!
    // all of this is assuming agents are on screen, todo cull based on camera and do different for offscreen agents
    // Update agents with move vectors (using results from background jobs)
    for (auto &agent : agents)
    {
        if (agent)
        {
            // Always use the last computed background move vector
            // This allows agents to keep moving while their next job is being processed
            v2 moveVector = agent->getBackgroundMoveVector();

            agent->update(dt, moveVector);
        }
    }

    // Update spatial grid with new positions
    updateSpatialGrid();

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

void LevelV1::renderAgentActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player)
{
    // Render player's action hitbox first if provided
    if (player)
    {
        const_cast<AnimatedDataCharacter *>(player)->renderActionHitbox();
    }

    // Use spatial grid to only check agents in visible cells
    CF_Aabb viewBounds = camera.getViewBounds();

    // Expand view bounds slightly to catch agents on the edge
    viewBounds.min.x -= 64.0f;
    viewBounds.min.y -= 64.0f;
    viewBounds.max.x += 64.0f;
    viewBounds.max.y += 64.0f;

    // Query spatial grid for agents in view area
    std::vector<size_t> nearbyAgents = spatialGrid.queryAABB(viewBounds);

    for (size_t agentIndex : nearbyAgents)
    {
        if (agentIndex >= agents.size())
            continue;

        auto &agent = agents[agentIndex];
        if (agent && agent->getIsOnScreen())
        {
            // Render action hitbox if agent is doing an action
            // Don't render during cooldown phase
            if (agent->getIsDoingAction() && agent->getActiveAction() && !agent->getActiveAction()->getInCooldown())
            {
                Action *action = agent->getActiveAction();

                // Calculate blended color from yellow to red based on warmup progress
                CF_Color yellow = cf_make_color_rgb(200, 200, 0);
                CF_Color red = cf_make_color_rgb(255, 0, 0);

                // Get warmup time from action JSON (in ms, convert to seconds)
                float warmupMs = action->contains("warmup") ? (*action)["warmup"].get<float>() : 0.0f;
                float warmupTime = warmupMs / 1000.0f;

                // Get current warmup timer
                float currentWarmupTime = action->getWarmupTimer();

                CF_Color blendedColor = blend(yellow, red, warmupTime, currentWarmupTime);

                agent->getActiveAction()->renderHitbox(blendedColor);
            }
        }
    }
}

void LevelV1::renderAgents(const CFNativeCamera &camera)
{
    int renderedCount = 0;
    int culledCount = 0;
    int checkedCount = 0;

    // Use spatial grid to only check agents in visible cells
    CF_Aabb viewBounds = camera.getViewBounds();

    // Expand view bounds slightly to catch agents on the edge
    viewBounds.min.x -= 64.0f;
    viewBounds.min.y -= 64.0f;
    viewBounds.max.x += 64.0f;
    viewBounds.max.y += 64.0f;

    // Query spatial grid for agents in view area
    std::vector<size_t> nearbyAgents = spatialGrid.queryAABB(viewBounds);
    checkedCount = static_cast<int>(nearbyAgents.size());

    for (size_t agentIndex : nearbyAgents)
    {
        if (agentIndex >= agents.size())
            continue;

        auto &agent = agents[agentIndex];
        if (agent)
        {
            // Check if agent is marked as on-screen by OnScreenChecks worker
            if (agent->getIsOnScreen())
            {
                v2 agentPos = agent->getPosition();
                agent->render(agentPos);
                renderedCount++;
            }
            else
            {
                culledCount++;
            }
        }
    }

    // Debug output: show how many agents were checked vs total
    // printf("LevelV1: checked %d/%zu agents, rendered: %d, culled: %d\n",
    //        checkedCount, agents.size(), renderedCount, culledCount);
}

void LevelV1::renderLayers(const CFNativeCamera &camera, const DataFile &config, float worldX, float worldY)
{
    if (!initialized || !levelMap)
    {
        return;
    }

    levelMap->renderAllLayers(camera, config, worldX, worldY);
}

void LevelV1::render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player, float worldX, float worldY)
{
    // Render in proper layer order:
    // 1. Level tiles that never change
    renderLayers(camera, config, worldX, worldY);

    // 2. Action hitboxes (player and agents)
    renderAgentActions(camera, player);

    // 3. All objects sorted by world Y position (structures, agents, player)
    renderedObjects.sort();

    renderedObjects.forEach([&](ObjectRenderedByWorldPosition &obj)
                            {
        if (obj.getType() == 1) // NavMeshAgent
        {
            auto agent = obj.asNavMeshAgent();
            if (agent && agent->getIsOnScreen())
            {
                v2 agentPos = agent->getPosition();
                agent->render(agentPos);
            }
        }
        else if (obj.getType() == 2) // PlayerCharacter
        {
            auto playerChar = obj.asPlayerCharacter();
            if (playerChar)
            {
                v2 playerPos = playerChar->getPosition();
                playerChar->render(playerPos);
            }
        }
        else if (obj.getType() == 0) // StructureLayer
        {
            auto structure = obj.asStructureLayer();
            if (structure && structure->getTMXLayer())
            {
                // Render the structure layer using the level map's renderSingleLayer method
                levelMap->renderSingleLayer(structure->getTMXLayer(), camera, config, worldX, worldY);
            }
        } });
}

void LevelV1::debugPrint() const
{
    printf("=== LevelV1 Debug Info ===\n");
    printf("  Directory: %s\n", levelDirectory.c_str());
    printf("  Name: %s\n", levelName.c_str());
    printf("  Initialized: %s\n", initialized ? "yes" : "no");
    printf("  Tile Size: %dx%d\n", tileWidth, tileHeight);

    if (levelMap)
    {
        printf("  TMX Map: loaded\n");
    }
    else
    {
        printf("  TMX Map: NOT loaded\n");
    }

    if (navmesh)
    {
        printf("  NavMesh: %d polygons, %d points\n",
               navmesh->getPolygonCount(), navmesh->getPointCount());
    }
    else
    {
        printf("  NavMesh: NOT created\n");
    }

    printf("  Agents: %zu\n", agents.size());
    for (size_t i = 0; i < agents.size(); ++i)
    {
        if (agents[i])
        {
            v2 pos = agents[i]->getPosition();
            printf("    Agent %zu: pos=(%.1f, %.1f), polygon=%d, walkable=%s\n",
                   i, pos.x, pos.y,
                   agents[i]->getCurrentPolygon(),
                   agents[i]->isOnWalkableArea() ? "yes" : "no");
        }
    }

    printf("  Entities: %s\n", entities.dump(2).c_str());
    printf("  Details: %s\n", details.dump(2).c_str());
    printf("========================\n");
}

bool LevelV1::checkAgentsInArea(const std::vector<CF_Aabb> &areas, CF_Aabb areasBounds,
                                const AnimatedDataCharacter *excludeAgent) const
{
    // Use spatial grid to narrow down which agents to check
    std::vector<size_t> nearbyAgents = spatialGrid.queryAABB(areasBounds);

    for (size_t agentIndex : nearbyAgents)
    {
        if (agentIndex >= agents.size())
            continue;

        const auto &agent = agents[agentIndex];
        if (!agent)
            continue;

        // Skip the excluded agent
        // AnimatedDataCharacterNavMeshAgent inherits from AnimatedDataCharacter, so we can compare directly
        if (excludeAgent && static_cast<const AnimatedDataCharacter *>(agent.get()) == excludeAgent)
            continue;

        // Get agent position and create a simple AABB for the agent
        v2 agentPos = agent->getPosition();
        float agentRadius = 32.0f; // Approximate agent size
        CF_Aabb agentBox = cf_make_aabb(
            cf_v2(agentPos.x - agentRadius, agentPos.y - agentRadius),
            cf_v2(agentPos.x + agentRadius, agentPos.y + agentRadius));

        // Check if agent overlaps with any of the areas
        for (const auto &area : areas)
        {
            if (cf_overlaps(area, agentBox))
            {
                return true;
            }
        }
    }

    return false;
}

void LevelV1::setPlayer(const AnimatedDataCharacter *playerCharacter)
{
    // Remove old player from rendered objects list if it exists
    if (player)
    {
        renderedObjects.remove(ObjectRenderedByWorldPosition(const_cast<AnimatedDataCharacter *>(player)));
    }

    player = playerCharacter;

    // Add new player to rendered objects list
    if (player)
    {
        renderedObjects.add(ObjectRenderedByWorldPosition(const_cast<AnimatedDataCharacter *>(player)));
        printf("LevelV1: Added player to rendered objects list (total: %zu)\n", renderedObjects.getCount());
    }
}

std::vector<AnimatedDataCharacterNavMeshAgent *> LevelV1::get_entities_at(int tile_x, int tile_y) const
{
    std::vector<AnimatedDataCharacterNavMeshAgent *> entities_at_tile;

    // Convert tile coordinates to world coordinates using same logic as highlightTile
    // tile_x: 0 = left column, increases rightward
    // tile_y: 0 = bottom row, increases upward (rendering convention)
    float tile_center_x = tile_x * static_cast<float>(tileWidth);
    float tile_center_y = tile_y * static_cast<float>(tileHeight);

    float half_width = static_cast<float>(tileWidth) / 2.0f;
    float half_height = static_cast<float>(tileHeight) / 2.0f;

    // Create the tile bounds
    CF_Aabb tile_bounds = make_aabb(
        cf_v2(tile_center_x - half_width, tile_center_y - half_height),
        cf_v2(tile_center_x + half_width, tile_center_y + half_height));

    // Query spatial grid for agents in this tile area
    std::vector<size_t> nearbyAgents = spatialGrid.queryAABB(tile_bounds);

    // Check each nearby agent to see if they're actually in the tile bounds
    for (size_t agentIndex : nearbyAgents)
    {
        if (agentIndex >= agents.size())
            continue;

        const auto &agent = agents[agentIndex];
        v2 agentPos = agent->getPosition();

        // Check if agent position is within tile bounds
        if (cf_contains_point(tile_bounds, agentPos))
        {
            entities_at_tile.push_back(agent.get());
        }
    }

    return entities_at_tile;
}

bool LevelV1::isCharacterInActionHitbox(const AnimatedDataCharacter *character, CF_Aabb characterBox) const
{
    // Check player's action if player exists and is doing an action
    if (player && player != character && player->getIsDoingAction() && player->getActiveAction())
    {
        Action *action = player->getActiveAction();
        if (!action->getInCooldown() && action->getHitBox())
        {
            std::vector<CF_Aabb> actionBoxes = action->getHitBox()->getBoxes(
                player->getCurrentDirection(), player->getPosition());

            for (const auto &actionBox : actionBoxes)
            {
                if (cf_overlaps(characterBox, actionBox))
                {
                    return true;
                }
            }
        }
    }

    // Check all agent actions
    for (size_t i = 0; i < agents.size(); ++i)
    {
        const auto &agent = agents[i];
        if (agent && agent.get() != character && agent->getIsDoingAction() && agent->getActiveAction())
        {
            Action *action = agent->getActiveAction();
            if (!action->getInCooldown() && action->getHitBox())
            {
                std::vector<CF_Aabb> actionBoxes = action->getHitBox()->getBoxes(
                    agent->getCurrentDirection(), agent->getPosition());

                for (const auto &actionBox : actionBoxes)
                {
                    if (cf_overlaps(characterBox, actionBox))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void LevelV1::updateSpatialGrid()
{
    // For simplicity, rebuild the grid each frame
    // This is efficient enough for hundreds of agents
    // For thousands, we'd want to track previous positions and use update()
    rebuildSpatialGrid();
}

void LevelV1::rebuildSpatialGrid()
{
    spatialGrid.clear();

    for (size_t i = 0; i < agents.size(); ++i)
    {
        if (agents[i])
        {
            v2 pos = agents[i]->getPosition();
            spatialGrid.insert(i, pos, 32.0f);
        }
    }
}
