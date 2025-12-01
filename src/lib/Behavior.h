#pragma once

#include "NavMesh.h"
#include "NavMeshPath.h"

// Base class for AI behaviors
class Behavior
{
public:
    Behavior();
    virtual ~Behavior();

    // Get a new navigation path based on the behavior
    // currentPosition: the current position of the agent
    // radius: the search radius for pathfinding
    // Returns a path through the navmesh (may be invalid if no path found)
    virtual std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition, int radius);
};
