#include "World.h"
#include <cstdio>

// WorldLevel implementation
WorldLevel::WorldLevel(const std::string &path, float x, float y)
    : folderPath(path), xOffset(x), yOffset(y), isOnScreen(false)
{
    printf("WorldLevel: Creating level from: %s at offset (%.1f, %.1f)\n", path.c_str(), x, y);
    level = std::make_unique<LevelV2>(path);

    if (!level->isInitialized())
    {
        printf("WorldLevel Error: Failed to initialize level from: %s\n", path.c_str());
    }
}

// World implementation
World::World(const std::string &directoryPath)
    : worldDirectory(directoryPath), initialized(false)
{
    printf("World: Loading world from directory: %s\n", directoryPath.c_str());

    // Load levels.json
    std::string levelsPath = directoryPath + "/levels.json";
    DataFile levelsData;

    try
    {
        levelsData = DataFile(levelsPath);
        printf("World: Loaded levels.json from: %s\n", levelsPath.c_str());
    }
    catch (const std::exception &e)
    {
        printf("World Error: Could not load levels.json: %s\n", e.what());
        return;
    }

    // Check if levels array exists
    if (!levelsData.contains("levels") || !levelsData["levels"].is_array())
    {
        printf("World Error: levels.json missing 'levels' array\n");
        return;
    }

    // Load each level
    const auto &levelsArray = levelsData["levels"];
    printf("World: Found %zu levels to load\n", levelsArray.size());

    for (const auto &levelEntry : levelsArray)
    {
        // Check required fields
        if (!levelEntry.contains("folder"))
        {
            printf("World Warning: Level entry missing 'folder' field, skipping\n");
            continue;
        }

        std::string folder = levelEntry["folder"].get<std::string>();
        float xOffset = levelEntry.contains("x_offset") ? levelEntry["x_offset"].get<float>() : 0.0f;
        float yOffset = levelEntry.contains("y_offset") ? levelEntry["y_offset"].get<float>() : 0.0f;

        printf("World: Loading level: %s (offset: %.1f, %.1f)\n", folder.c_str(), xOffset, yOffset);

        // Create and add the WorldLevel
        levels.emplace_back(folder, xOffset, yOffset);

        // Check if level initialized successfully
        if (!levels.back().level->isInitialized())
        {
            printf("World Error: Failed to initialize level: %s\n", folder.c_str());
            levels.pop_back(); // Remove the failed level
            continue;
        }

        printf("World: Successfully loaded level: %s\n", folder.c_str());
    }

    if (levels.empty())
    {
        printf("World Error: No levels were successfully loaded\n");
        return;
    }

    initialized = true;
    printf("World: Successfully initialized with %zu levels\n", levels.size());

    // Build combined world navmesh from all levels
    navmesh = std::make_unique<NavMesh>();

    // Combine navmeshes from all levels into the world navmesh
    // Each level's navmesh needs to be offset by the level's world position
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            NavMesh &levelNavmesh = worldLevel.level->getNavMesh();

            // Copy all polygons from level navmesh with position offset
            for (int i = 0; i < levelNavmesh.getPolygonCount(); ++i)
            {
                // Get polygon points from level navmesh
                // Note: This is a simplified version - actual implementation would need
                // to properly merge the navmesh data structures with offsets
                // For now, we'll just note this needs deeper integration
                printf("World: TODO - Properly merge navmesh from level %s with offset (%.1f, %.1f)\n",
                       worldLevel.folderPath.c_str(), worldLevel.xOffset, worldLevel.yOffset);
            }
        }
    }

    printf("World: Built combined navmesh with %d polygons\n", navmesh->getPolygonCount());

    // Build initial world spatial grid with all agents from all levels
    rebuildSpatialGrid();
}

WorldLevel &World::getLevel(size_t index)
{
    if (index >= levels.size())
    {
        printf("World Error: Level index %zu out of bounds (size: %zu)\n", index, levels.size());
        throw std::out_of_range("Level index out of bounds");
    }
    return levels[index];
}

const WorldLevel &World::getLevel(size_t index) const
{
    if (index >= levels.size())
    {
        printf("World Error: Level index %zu out of bounds (size: %zu)\n", index, levels.size());
        throw std::out_of_range("Level index out of bounds");
    }
    return levels[index];
}

void World::updateLevelVisibility(const CF_Aabb &viewBounds)
{
    for (auto &worldLevel : levels)
    {
        if (!worldLevel.level)
            continue;

        // Get level bounds in world space
        const LevelMap &levelMap = worldLevel.level->getLevelMap();
        int mapWidth = levelMap.getMapWidth();
        int mapHeight = levelMap.getMapHeight();
        int tileWidth = worldLevel.level->getTileWidth();
        int tileHeight = worldLevel.level->getTileHeight();

        // Calculate level bounds with offsets
        float levelMinX = worldLevel.xOffset;
        float levelMinY = worldLevel.yOffset;
        float levelMaxX = worldLevel.xOffset + (mapWidth * tileWidth);
        float levelMaxY = worldLevel.yOffset + (mapHeight * tileHeight);

        CF_Aabb levelBounds = cf_make_aabb(
            cf_v2(levelMinX, levelMinY),
            cf_v2(levelMaxX, levelMaxY));

        // Check if level intersects with view bounds
        bool wasOnScreen = worldLevel.isOnScreen;
        worldLevel.isOnScreen = cf_overlaps(viewBounds, levelBounds);

        if (worldLevel.isOnScreen != wasOnScreen)
        {
            printf("World: Level %s visibility changed: %s\n",
                   worldLevel.folderPath.c_str(),
                   worldLevel.isOnScreen ? "on-screen" : "off-screen");
        }
    }
}

// Agent management methods

size_t World::getAgentCount() const
{
    size_t count = 0;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            count += worldLevel.level->getAgentCount();
        }
    }
    return count;
}

size_t World::getOnscreenAgentCount() const
{
    size_t count = 0;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            count += worldLevel.level->getAgentCount();
        }
    }
    return count;
}

size_t World::getOffscreenAgentCount() const
{
    size_t count = 0;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            count += worldLevel.level->getAgentCount();
        }
    }
    return count;
}

std::vector<AnimatedDataCharacterNavMeshAgent *> World::getAgents()
{
    std::vector<AnimatedDataCharacterNavMeshAgent *> allAgents;
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                allAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return allAgents;
}

std::vector<const AnimatedDataCharacterNavMeshAgent *> World::getAgents() const
{
    std::vector<const AnimatedDataCharacterNavMeshAgent *> allAgents;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                allAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return allAgents;
}

std::vector<AnimatedDataCharacterNavMeshAgent *> World::getOnscreenAgents()
{
    std::vector<AnimatedDataCharacterNavMeshAgent *> onscreenAgents;
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                onscreenAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return onscreenAgents;
}

std::vector<const AnimatedDataCharacterNavMeshAgent *> World::getOnscreenAgents() const
{
    std::vector<const AnimatedDataCharacterNavMeshAgent *> onscreenAgents;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                onscreenAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return onscreenAgents;
}

std::vector<AnimatedDataCharacterNavMeshAgent *> World::getOffscreenAgents()
{
    std::vector<AnimatedDataCharacterNavMeshAgent *> offscreenAgents;
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                offscreenAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return offscreenAgents;
}

std::vector<const AnimatedDataCharacterNavMeshAgent *> World::getOffscreenAgents() const
{
    std::vector<const AnimatedDataCharacterNavMeshAgent *> offscreenAgents;
    for (const auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                offscreenAgents.push_back(worldLevel.level->getAgent(i));
            }
        }
    }
    return offscreenAgents;
}

void World::setPlayer(const AnimatedDataCharacter *player)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            worldLevel.level->setPlayer(player);
        }
    }
}

void World::clearAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            worldLevel.level->clearAgents();
        }
    }
    printf("World: Cleared all agents from all levels\n");
}

void World::clearOnscreenAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->clearAgents();
        }
    }
    printf("World: Cleared agents from onscreen levels\n");
}

void World::clearOffscreenAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            worldLevel.level->clearAgents();
        }
    }
    printf("World: Cleared agents from offscreen levels\n");
}

// Update methods

void World::updateAgents(float dt)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            worldLevel.level->updateAgents(dt);
        }
    }
}

void World::updateOnscreenAgents(float dt)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->updateAgents(dt);
        }
    }
}

void World::updateOffscreenAgents(float dt)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            worldLevel.level->updateAgents(dt);
        }
    }
}

void World::updateSpatialGrid()
{
    // For simplicity, rebuild the grid each frame
    // This is efficient enough for hundreds of agents
    rebuildSpatialGrid();
}

void World::rebuildSpatialGrid()
{
    spatialGrid.clear();

    // Add all agents from all levels to the world spatial grid
    size_t globalAgentIndex = 0;
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            for (size_t i = 0; i < worldLevel.level->getAgentCount(); ++i)
            {
                AnimatedDataCharacterNavMeshAgent *agent = worldLevel.level->getAgent(i);
                if (agent)
                {
                    // Get agent position (already in world coordinates)
                    v2 pos = agent->getPosition();

                    // Insert into world spatial grid with global index
                    spatialGrid.insert(globalAgentIndex, pos, 32.0f);
                    globalAgentIndex++;
                }
            }
        }
    }

    printf("World: Rebuilt spatial grid with %zu agents\n", globalAgentIndex);
}

void World::cullDyingAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level)
        {
            worldLevel.level->cullDyingAgents();
        }
    }
}

void World::cullOnscreenDyingAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->cullDyingAgents();
        }
    }
}

void World::cullOffscreenDyingAgents()
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && !worldLevel.isOnScreen)
        {
            worldLevel.level->cullDyingAgents();
        }
    }
}

// Rendering methods

void World::renderLayers(const CFNativeCamera &camera, const DataFile &config)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->renderLayers(camera, config, worldLevel.xOffset, worldLevel.yOffset);
        }
    }
}

void World::renderAgentActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->renderAgentActions(camera, player);
        }
    }
}

void World::renderPlayerAvailableActions(const CFNativeCamera &camera, const AnimatedDataCharacter *player)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->renderPlayerAvailableActions(camera, player);
        }
    }
}

void World::renderAgents(const CFNativeCamera &camera)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->renderAgents(camera);
        }
    }
}

void World::render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player)
{
    for (auto &worldLevel : levels)
    {
        if (worldLevel.level && worldLevel.isOnScreen)
        {
            worldLevel.level->render(camera, config, player, worldLevel.xOffset, worldLevel.yOffset);
        }
    }
}

void World::debugPrint() const
{
    printf("=== World Debug Info ===\n");
    printf("  Directory: %s\n", worldDirectory.c_str());
    printf("  Initialized: %s\n", initialized ? "yes" : "no");
    printf("  Levels: %zu\n", levels.size());
    printf("  Total Agents: %zu\n", getAgentCount());
    printf("  Onscreen Agents: %zu\n", getOnscreenAgentCount());
    printf("  Offscreen Agents: %zu\n", getOffscreenAgentCount());

    for (size_t i = 0; i < levels.size(); ++i)
    {
        const auto &worldLevel = levels[i];
        printf("  Level %zu:\n", i);
        printf("    Folder: %s\n", worldLevel.folderPath.c_str());
        printf("    Offset: (%.1f, %.1f)\n", worldLevel.xOffset, worldLevel.yOffset);
        printf("    On Screen: %s\n", worldLevel.isOnScreen ? "yes" : "no");

        if (worldLevel.level)
        {
            printf("    Level Initialized: %s\n", worldLevel.level->isInitialized() ? "yes" : "no");
            printf("    Level Name: %s\n", worldLevel.level->getLevelName().c_str());
            printf("    Agents: %zu\n", worldLevel.level->getAgentCount());
        }
        else
        {
            printf("    Level: nullptr\n");
        }
    }

    printf("========================\n");
}
