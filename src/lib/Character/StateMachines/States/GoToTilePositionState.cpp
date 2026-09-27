#include "GoToTilePositionState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

GoToTilePositionState::GoToTilePositionState()
    : State(), tile_x(0), tile_y(0), hasGeneratedPath(false)
{
}

GoToTilePositionState::~GoToTilePositionState()
{
}

void GoToTilePositionState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'tile_x' and 'tile_y' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("tile_x") && defaults["tile_x"].is_number())
    {
        tile_x = defaults["tile_x"].get<int>();
    }
    if (defaults.contains("tile_y") && defaults["tile_y"].is_number())
    {
        tile_y = defaults["tile_y"].get<int>();
    }
}

void GoToTilePositionState::update(float dt)
{
    // This state does nothing on update
    // It only generates a path when GetNewPath is called
}

std::shared_ptr<NavMeshPath> GoToTilePositionState::GetNewPath(NavMesh &navmesh, CF_V2 currentPosition)
{
    // Convert tile coordinates to pixel coordinates using TILE_SIZE
    const int TILE_SIZE = 32; // Adjust based on your actual tile size

    // Calculate target position from tile_x and tile_y coordinates
    CF_V2 targetPosition = cf_v2(static_cast<float>(tile_x * TILE_SIZE), static_cast<float>(tile_y * TILE_SIZE));

    // Mark that we've generated a path, so set isRunning to false
    setIsRunning(false);

    // Check if the target position is walkable on the navmesh
    if (!navmesh.isWalkable(targetPosition))
    {
        printf("GoToTilePositionState: Target tile (%d, %d) is not reachable on navmesh\n", tile_x, tile_y);
        return nullptr;
    }

    // Try to generate a path; return nullptr if no valid path could be found
    std::shared_ptr<NavMeshPath> path = navmesh.generatePath(currentPosition, targetPosition);
    if (!path || !path->isValid())
    {
        printf("GoToTilePositionState: Failed to generate a path to tile (%d, %d)\n", tile_x, tile_y);
        return nullptr;
    }

    printf("GoToTilePositionState: Generated path to (%.2f, %.2f)\n", targetPosition.x, targetPosition.y);
    return path;
}

int GoToTilePositionState::getTileX() const
{
    return tile_x;
}

int GoToTilePositionState::getTileY() const
{
    return tile_y;
}
