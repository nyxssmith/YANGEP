#include "GetNewPositionFromCoordinatorState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

GetNewPositionFromCoordinatorState::GetNewPositionFromCoordinatorState()
    : State(), hasGeneratedPath(false)
{
}

GetNewPositionFromCoordinatorState::~GetNewPositionFromCoordinatorState()
{
}

void GetNewPositionFromCoordinatorState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // todo get the coordinator from the agent's level
}

void GetNewPositionFromCoordinatorState::update(float dt)
{
    // This state does nothing on update
    // It only generates a path when GetNewPath is called
}

std::shared_ptr<NavMeshPath> GetNewPositionFromCoordinatorState::GetNewPath(NavMesh &navmesh, CF_V2 currentPosition)
{
    // Convert tiles to pixel radius (assuming standard tile size)
    // We'll use a tile size estimate - you may want to pass this in
    const int TILE_SIZE = 32; // Adjust based on your actual tile size
    // TODO get tile size from level at init

    // todo get the target position from the coordinator
    CF_V2 targetPosition = cf_v2(0, 0);

    // Check if the point is on the navmesh
    if (navmesh.isWalkable(targetPosition))
    {
        // Mark that we've generated a path, so set isRunning to false
        setIsRunning(false);

        return navmesh.generatePath(currentPosition, targetPosition);
    }

    // Return an empty/invalid path
    // printf("WanderNewPositionState: Failed to find valid wander position\n");
    return std::make_shared<NavMeshPath>();
}
