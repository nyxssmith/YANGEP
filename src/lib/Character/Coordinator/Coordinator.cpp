#include "Coordinator.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "LevelV1.h"
#include <algorithm>
#include <stdio.h>

Coordinator::Coordinator()
    : m_nearPlayerTileGrid(7), m_player(nullptr), m_level(nullptr), m_lastPlayerTileX(INT_MIN), m_lastPlayerTileY(INT_MIN), m_agentListChanged(false), m_lastUpdateTimeMs(0.0) // Default to 7x7 grid
{
    printf("Coordinator: Initialized\n");
}

Coordinator::~Coordinator()
{
    clear();
    printf("Coordinator: Destroyed\n");
}

void Coordinator::initialize(const AnimatedDataCharacter *player, LevelV1 *level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_player = player;
    m_level = level;
    printf("Coordinator: Initialized with player=%p, level=%p\n", (void *)player, (void *)level);
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
    m_agentListChanged = true;
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

    // Clear any tiles in the near-player grid claimed by this agent
    int halfSize = m_nearPlayerTileGrid.getGridSize() / 2;
    for (int ny = -halfSize; ny <= halfSize; ++ny)
    {
        for (int nx = -halfSize; nx <= halfSize; ++nx)
        {
            NearPlayerTile *tile = m_nearPlayerTileGrid.getTileMutable(nx, ny);
            if (tile && tile->agent == agent)
            {
                tile->status = TileStatus::Empty;
                tile->agent = nullptr;
            }
        }
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
    m_agentListChanged = true;
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

void Coordinator::update()
{
    auto startTime = std::chrono::high_resolution_clock::now();

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_player || !m_level)
    {
        return;
    }

    // Check if player has moved to a different tile
    v2 playerPosition = m_player->getPosition();
    float tileWidth = static_cast<float>(m_level->getTileWidth());
    float tileHeight = static_cast<float>(m_level->getTileHeight());
    int currentPlayerTileX = static_cast<int>(std::round(playerPosition.x / tileWidth));
    int currentPlayerTileY = static_cast<int>(std::round(playerPosition.y / tileHeight));

    // Always update the near-player grid to keep it centered on the player
    m_nearPlayerTileGrid.updatePlayerPosition(playerPosition.x, playerPosition.y, currentPlayerTileX, currentPlayerTileY);

    // Early exit if player hasn't moved to a different tile AND agent list hasn't changed
    if (currentPlayerTileX == m_lastPlayerTileX && currentPlayerTileY == m_lastPlayerTileY && !m_agentListChanged)
    {
        return;
    }

    // Update last player tile position
    m_lastPlayerTileX = currentPlayerTileX;
    m_lastPlayerTileY = currentPlayerTileY;
    m_agentListChanged = false; // Reset the flag

    // STEP 1: Copy all agent data we need while holding the lock
    // This prevents race conditions with main thread deleting/modifying agents
    std::vector<AgentProcessData> agentDataList;
    agentDataList.reserve(m_agents.size());

    for (auto *agent : m_agents)
    {
        if (!agent)
        {
            continue;
        }

        AgentProcessData data;
        data.agent = agent;
        data.position = agent->getPosition();
        data.hasValidAction = false;

        // Calculate distance to player
        float dx = data.position.x - playerPosition.x;
        float dy = data.position.y - playerPosition.y;
        data.distSq = dx * dx + dy * dy;

        // Try to copy hitbox tiles if action exists
        Action *actionA = agent->getActionPointerA();
        if (actionA)
        {
            HitBox *hitbox = actionA->getHitBox();
            if (hitbox)
            {
                // Make a deep copy of the tiles vector
                const auto &tiles = hitbox->getTiles();
                if (!tiles.empty())
                {
                    data.hitboxTiles = tiles; // Vector copy
                    data.hasValidAction = true;
                    printf("Agent %p: Copied %zu hitbox tiles (dist: %.2f)\n",
                           (void *)agent, data.hitboxTiles.size(), std::sqrt(data.distSq));
                }
            }
        }

        agentDataList.push_back(data);
    }

    // STEP 2: Sort by distance (closest first)
    std::sort(agentDataList.begin(), agentDataList.end(),
              [](const AgentProcessData &a, const AgentProcessData &b)
              { return a.distSq < b.distSq; });

    // STEP 3: Process using copied data (no agent pointer dereferencing)
    for (const auto &data : agentDataList)
    {
        if (!data.hasValidAction || data.hitboxTiles.empty())
        {
            continue;
        }

        // Process with copied tile data - no accessing agent internals
        tryPlaceHitboxOnGrid(data.hitboxTiles, data.position, data.agent);
    }

    // get shapes that can be made from actions
    //  route those to overlap player, and then go to next agent and route that etc

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
    m_lastUpdateTimeMs = elapsed.count();
}

const NearPlayerTileGrid &Coordinator::getNearPlayerTileGrid() const
{
    return m_nearPlayerTileGrid;
}

NearPlayerTileGrid &Coordinator::getNearPlayerTileGridMutable()
{
    return m_nearPlayerTileGrid;
}

void Coordinator::setNearPlayerTileGridSize(int gridSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_nearPlayerTileGrid.initialize(gridSize);
}

const AnimatedDataCharacter *Coordinator::getPlayer() const
{
    return m_player;
}

LevelV1 *Coordinator::getLevel() const
{
    return m_level;
}

double Coordinator::getLastUpdateTimeMs() const
{
    return m_lastUpdateTimeMs;
}

void Coordinator::updateNearPlayerGrid(int playerTileX, int playerTileY)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_level)
    {
        return;
    }

    // Calculate world position from tile position
    float tileWidth = static_cast<float>(m_level->getTileWidth());
    float tileHeight = static_cast<float>(m_level->getTileHeight());
    float playerWorldX = playerTileX * tileWidth;
    float playerWorldY = playerTileY * tileHeight;

    // Update the grid
    m_nearPlayerTileGrid.updatePlayerPosition(playerWorldX, playerWorldY, playerTileX, playerTileY);
}

void Coordinator::render() const
{
    if (!m_level)
    {
        return;
    }

    // Render the near-player tile grid
    m_nearPlayerTileGrid.render(*m_level);
}

bool Coordinator::tryPlaceHitboxOnGrid(const std::vector<HitboxTile> &hitboxTiles, v2 agentPosition, AnimatedDataCharacterNavMeshAgent *agent)
{
    // NOTE: This method must be called with m_mutex held (called from update())
    // Uses copied tile data to avoid accessing potentially deleted agent/action/hitbox objects
    if (!m_level || !agent || hitboxTiles.empty())
    {
        return false;
    }

    printf("tryPlaceHitboxOnGrid: Processing %zu tiles for agent %p\n", hitboxTiles.size(), (void *)agent);

    // Get access to the grid
    NearPlayerTileGrid &grid = getNearPlayerTileGridMutable();

    // First, clear any existing tiles claimed by this agent
    int halfSize = grid.getGridSize() / 2;
    for (int ny = -halfSize; ny <= halfSize; ++ny)
    {
        for (int nx = -halfSize; nx <= halfSize; ++nx)
        {
            NearPlayerTile *tile = grid.getTileMutable(nx, ny);
            if (tile && tile->status != TileStatus::Empty && tile->agent == agent)
            {
                tile->status = TileStatus::Empty;
                tile->agent = nullptr;
                printf("  Cleared previous claim by agent at tile (%d, %d)\n", nx, ny);
            }
        }
    }

    // Check if there are ANY empty tiles in the grid
    bool hasEmptyTile = false;
    for (int ny = -halfSize; ny <= halfSize && !hasEmptyTile; ++ny)
    {
        for (int nx = -halfSize; nx <= halfSize; ++nx)
        {
            const NearPlayerTile *tile = grid.getTile(nx, ny);
            if (tile && tile->status == TileStatus::Empty)
            {
                hasEmptyTile = true;
                break;
            }
        }
    }

    if (!hasEmptyTile)
    {
        printf("tryPlaceHitboxOnGrid: No empty tiles in grid\n");
        return false;
    }

    if (hitboxTiles.empty())
    {
        return false;
    }

    // Build list of target tiles sorted by distance from player (0,0)
    // We want to prioritize hitting tiles near the player
    std::vector<std::pair<int, std::pair<int, int>>> targetTilesByDist;
    for (int ny = -halfSize; ny <= halfSize; ++ny)
    {
        for (int nx = -halfSize; nx <= halfSize; ++nx)
        {
            int distSq = nx * nx + ny * ny;
            targetTilesByDist.push_back({distSq, {nx, ny}});
        }
    }

    // Sort by distance (closest to player first)
    std::sort(targetTilesByDist.begin(), targetTilesByDist.end(),
              [](const auto &a, const auto &b)
              { return a.first < b.first; });

    // Calculate agent's current tile position
    float tileWidth = static_cast<float>(m_level->getTileWidth());
    float tileHeight = static_cast<float>(m_level->getTileHeight());
    int agentCurrentTileX = static_cast<int>(std::round(agentPosition.x / tileWidth));
    int agentCurrentTileY = static_cast<int>(std::round(agentPosition.y / tileHeight));

    // Get player tile position to convert to near-player coordinates
    const NearPlayerTile *playerTile = grid.getTile(0, 0);
    if (!playerTile)
    {
        return false;
    }
    int playerTileX = playerTile->tileX;
    int playerTileY = playerTile->tileY;

    // Convert agent's current position to near-player coordinates
    int agentCurrentNearX = agentCurrentTileX - playerTileX;
    int agentCurrentNearY = agentCurrentTileY - playerTileY;

    // Find which hitbox tile, target tile, agent position, and facing direction would hit near the player
    // while being closest to agent's current position
    // Try all target tiles (sorted by distance to player) and all 4 directions
    int bestAgentX = 0;
    int bestAgentY = 0;
    int bestTargetX = 0;
    int bestTargetY = 0;
    float bestAgentDistSq = -1.0f;
    const HitboxTile *bestHitboxTile = nullptr;
    Direction bestDirection = Direction::RIGHT;

    // Try each target tile starting from closest to player
    for (const auto &[targetDistSq, targetCoords] : targetTilesByDist)
    {
        int targetX = targetCoords.first;
        int targetY = targetCoords.second;

        for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            for (const auto &hitboxTile : hitboxTiles)
            {
                // Rotate the hitbox tile coordinates based on direction (tiles are defined for facing RIGHT)
                v2 rotated;
                switch (direction)
                {
                case Direction::RIGHT:
                    rotated = cf_v2((float)hitboxTile.x, (float)hitboxTile.y);
                    break;
                case Direction::UP:
                    rotated = cf_v2((float)-hitboxTile.y, (float)hitboxTile.x);
                    break;
                case Direction::LEFT:
                    rotated = cf_v2((float)-hitboxTile.x, (float)-hitboxTile.y);
                    break;
                case Direction::DOWN:
                    rotated = cf_v2((float)hitboxTile.y, (float)-hitboxTile.x);
                    break;
                }

                // Calculate where agent would need to be for this rotated hitbox tile to hit the target
                int candidateAgentX = targetX - (int)rotated.x;
                int candidateAgentY = targetY - (int)rotated.y;

                // Check if ALL tiles that this hitbox would occupy are available
                bool allTilesAvailable = true;
                for (const auto &checkTile : hitboxTiles)
                {
                    // Rotate this tile too
                    v2 checkRotated;
                    switch (direction)
                    {
                    case Direction::RIGHT:
                        checkRotated = cf_v2((float)checkTile.x, (float)checkTile.y);
                        break;
                    case Direction::UP:
                        checkRotated = cf_v2((float)-checkTile.y, (float)checkTile.x);
                        break;
                    case Direction::LEFT:
                        checkRotated = cf_v2((float)-checkTile.x, (float)-checkTile.y);
                        break;
                    case Direction::DOWN:
                        checkRotated = cf_v2((float)checkTile.y, (float)-checkTile.x);
                        break;
                    }

                    int checkX = candidateAgentX + (int)checkRotated.x;
                    int checkY = candidateAgentY + (int)checkRotated.y;

                    const NearPlayerTile *tile = grid.getTile(checkX, checkY);
                    if (!tile || tile->status != TileStatus::Empty)
                    {
                        allTilesAvailable = false;
                        break;
                    }
                }

                if (!allTilesAvailable)
                {
                    continue; // Some tiles for this placement are not available
                }

                // Also check if agent position is available
                const NearPlayerTile *agentPosTile = grid.getTile(candidateAgentX, candidateAgentY);
                if (!agentPosTile || agentPosTile->status != TileStatus::Empty)
                {
                    continue; // Agent position is not available
                }

                // Calculate distance from agent's current position
                int dx = candidateAgentX - agentCurrentNearX;
                int dy = candidateAgentY - agentCurrentNearY;
                float distSq = static_cast<float>(dx * dx + dy * dy);

                if (bestHitboxTile == nullptr || distSq < bestAgentDistSq)
                {
                    bestAgentX = candidateAgentX;
                    bestAgentY = candidateAgentY;
                    bestTargetX = targetX;
                    bestTargetY = targetY;
                    bestAgentDistSq = distSq;
                    bestHitboxTile = &hitboxTile;
                    bestDirection = direction;
                }
            }
        }

        // If we found a valid placement, stop searching (we prioritize hitting closer to player)
        if (bestHitboxTile != nullptr)
        {
            break;
        }
    }

    if (!bestHitboxTile)
    {
        printf("tryPlaceHitboxOnGrid: No valid agent position found to hit player\n");
        return false;
    }

    printf("  Best hitbox tile relative position: (%d, %d)\n", bestHitboxTile->x, bestHitboxTile->y);
    printf("  Best direction: %d (0=Up, 1=Down, 2=Left, 3=Right)\n", (int)bestDirection);
    printf("  Best target tile: (%d, %d), distance from player: %.2f\n", bestTargetX, bestTargetY, std::sqrt((float)(bestTargetX * bestTargetX + bestTargetY * bestTargetY)));
    printf("  Agent current position: (%d, %d), best position: (%d, %d), distance: %.2f\n",
           agentCurrentNearX, agentCurrentNearY, bestAgentX, bestAgentY, std::sqrt(bestAgentDistSq));

    // Mark ALL hitbox tiles as PlannedAction (rotated for the best direction)
    for (const auto &hitboxTile : hitboxTiles)
    {
        // Rotate the hitbox tile coordinates based on bestDirection
        v2 rotated;
        switch (bestDirection)
        {
        case Direction::RIGHT:
            rotated = cf_v2((float)hitboxTile.x, (float)hitboxTile.y);
            break;
        case Direction::UP:
            rotated = cf_v2((float)-hitboxTile.y, (float)hitboxTile.x);
            break;
        case Direction::LEFT:
            rotated = cf_v2((float)-hitboxTile.x, (float)-hitboxTile.y);
            break;
        case Direction::DOWN:
            rotated = cf_v2((float)hitboxTile.y, (float)-hitboxTile.x);
            break;
        }

        // Calculate the actual tile position in near-player grid
        int actionTileX = bestAgentX + (int)rotated.x;
        int actionTileY = bestAgentY + (int)rotated.y;

        NearPlayerTile *actionTile = grid.getTileMutable(actionTileX, actionTileY);
        if (actionTile)
        {
            actionTile->status = TileStatus::PlannedAction;
            actionTile->agent = agent;
            printf("  Marked tile (%d, %d) as PlannedAction for agent %p\n", actionTileX, actionTileY, (void *)agent);
        }
    }

    int agentX = bestAgentX;
    int agentY = bestAgentY;
    printf("  Agent should be at (%d, %d) to perform action\n", agentX, agentY);

    // Mark the agent's required tile as PlannedOccupiedByAgent
    NearPlayerTile *agentTile = grid.getTileMutable(agentX, agentY);
    if (agentTile)
    {
        agentTile->status = TileStatus::PlannedOccupiedByAgent;
        agentTile->agent = agent;
        printf("  Marked tile (%d, %d) as PlannedOccupiedByAgent for agent %p\n", agentX, agentY, (void *)agent);
    }

    return true;
}
