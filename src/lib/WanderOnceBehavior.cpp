#include "WanderOnceBehavior.h"
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// Constructor
WanderOnceBehavior::WanderOnceBehavior()
    : WanderBehavior()
{
    // Seed random number generator (only once per behavior instance)
    static bool seeded = false;
    if (!seeded)
    {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
}

// Destructor
WanderOnceBehavior::~WanderOnceBehavior()
{
}

// Get a new random wander path
std::shared_ptr<NavMeshPath> WanderOnceBehavior::GetNewPath(NavMesh &navmesh, CF_V2 currentPosition, int radius)
{
    // DEBUG: 2 second wait
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    if (hasWandered)
    {
        // empty path
        return std::make_shared<NavMeshPath>();
    }
    // Maximum attempts to find a valid point on the navmesh
    const int maxAttempts = 20;
    CF_V2 targetPosition;
    bool foundValidPoint = false;

    // Try to find a random point within radius that is on the navmesh
    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        // Pick a random point within +/-radius in x and y directions
        float randomX = currentPosition.x + ((rand() % (radius * 2 + 1)) - radius);
        float randomY = currentPosition.y + ((rand() % (radius * 2 + 1)) - radius);
        targetPosition = cf_v2(randomX, randomY);

        // Check if the point is on the navmesh
        if (navmesh.isWalkable(targetPosition))
        {
            foundValidPoint = true;
            break;
        }
    }

    // If we found a valid point, generate a path to it using the navmesh
    if (foundValidPoint)
    {
        hasWandered = true;
        return navmesh.generatePath(currentPosition, targetPosition);
    }

    // Return an empty/invalid path
    return std::make_shared<NavMeshPath>();
}
