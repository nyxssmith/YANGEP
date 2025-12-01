#pragma once

#include "Behavior.h"
#include "WanderBehavior.h"

// Wander behavior - agent wanders randomly through the navmesh
class WanderOnceBehavior : public WanderBehavior
{
public:
    WanderOnceBehavior();
    ~WanderOnceBehavior();
    bool hasWandered = false;
    // Get a new random wander path
    std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition, int radius) override;
};
