#include "WanderNewPositionState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

WanderNewPositionState::WanderNewPositionState()
    : State(), tiles_radius(10), hasGeneratedPath(false)
{
    // Seed random number generator (only once)
    static bool seeded = false;
    if (!seeded)
    {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
}

WanderNewPositionState::~WanderNewPositionState()
{
}

void WanderNewPositionState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'tiles_radius' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("tiles_radius"))
    {
        if (defaults["tiles_radius"].is_number())
        {
            tiles_radius = defaults["tiles_radius"].get<int>();
            // printf("WanderNewPositionState: Loaded tiles_radius: %d\n", tiles_radius);
        }
    }
}

void WanderNewPositionState::update(float dt)
{
    // This state does nothing on update
    // It only generates a path when GetNewPath is called
}

std::shared_ptr<NavMeshPath> WanderNewPositionState::GetNewPath(NavMesh &navmesh, CF_V2 currentPosition)
{
    // Convert tiles to pixel radius (assuming standard tile size)
    // We'll use a tile size estimate - you may want to pass this in
    const int TILE_SIZE = 32; // Adjust based on your actual tile size
    int radiusPixels = tiles_radius * TILE_SIZE;

    // Maximum attempts to find a valid point on the navmesh
    const int maxAttempts = 20;
    CF_V2 targetPosition;
    bool foundValidPoint = false;

    // Try to find a random point within radius that is on the navmesh
    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        // Pick a random point within +/-radius in x and y directions
        float randomX = currentPosition.x + ((rand() % (radiusPixels * 2 + 1)) - radiusPixels);
        float randomY = currentPosition.y + ((rand() % (radiusPixels * 2 + 1)) - radiusPixels);
        targetPosition = cf_v2(randomX, randomY);

        // Check if the point is on the navmesh
        if (navmesh.isWalkable(targetPosition))
        {
            foundValidPoint = true;
            break;
        }
    }

    // Mark that we've generated a path, so set isRunning to false
    setIsRunning(false);

    // If we found a valid point, generate a path to it using the navmesh
    if (foundValidPoint)
    {
        // printf("WanderNewPositionState: Generated path to (%.2f, %.2f)\n", targetPosition.x, targetPosition.y);
        return navmesh.generatePath(currentPosition, targetPosition);
    }

    // Return an empty/invalid path
    // printf("WanderNewPositionState: Failed to find valid wander position\n");
    return std::make_shared<NavMeshPath>();
}

int WanderNewPositionState::getTilesRadius() const
{
    return tiles_radius;
}
