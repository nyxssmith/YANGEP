#include "WanderBehavior.h"
#include <cstdlib>
#include <ctime>

// Constructor
WanderBehavior::WanderBehavior()
    : Behavior()
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
WanderBehavior::~WanderBehavior()
{
}

// Get a new random wander path
NavMeshPath WanderBehavior::GetNewPath(const NavMesh &navmesh, CF_V2 currentPosition, int radius)
{
    NavMeshPath path;

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

    // If we found a valid point, generate a path to it
    if (foundValidPoint)
    {
        path.generate(navmesh, currentPosition, targetPosition);
    }

    return path;
}
